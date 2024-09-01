/*
 * main.c
 *
*/

/*
    From ws2812b_dma_spi_led_driver.h:
    **For the CH32V003 this means output will be on PORTC Pin 6**

    From i2c_slave.h:
        SDA and SCL [PC1 and PC2].
    
    Programming:
        PD1 is SWIO.
    
    Timer interrupt:
        TIM1C1 uses PD2.

    Button
        PC3 - TBD
        PC4 - TBD

    LED(s):
        PC0 (@debug for now)
    
    For UART printf, on:
		CH32V003, Port D5, 115200 8n1
*/
#include "main.h"
#include <stdio.h>

#include <ch32v003fun.h>
#include <ch32v003_GPIO_branchless.h>
#include <i2c_slave.h>

#define LEDS 16
#define MIN_LED 0
// This is default in .h but making it explicit. > Has to be divisible by 4.
#define DMALEDS 20 // For lib, for testing.

#define WS2812DMA_IMPLEMENTATION
#define WSRBG

/* @todo registers:
    0-15: reserverd, 0 is version # - not writtable
    16-31: reserved for gloabal settings
    32-47: stars
    48-63: eyes
    64-95: upper trim
    96-127: lower trim
*/

#include <ws2812b_dma_spi_led_driver.h>

volatile uint8_t timer_tick = 0;

// @debug testing.
struct WsEffect comet = {
    "comet",
    "Comet",
    250,
    250,
    250,
    (uint32_t) 0xC00000,
    1,
    1,
    1,
    1,   
};

/*
struct WsSections ws_section_eyes = {
    "eyes",
    WS_EYES_LED_START,
    WS_EYES_LED_END,
    WS_EYES_LED_COUNT,
};
*/

/*
struct WsSections ws_section_upper_trim = {
    "upper_trim",
    WS_UPPER_TRIM_START,
    WS_UPPER_TRIM_END,
    WS_UPPER_TRIM_COUNT
};
*/

/*
struct WsSections ws_section_lower_trim = {
    "lower_trim",
    WS_LOWER_TRIM_START,
    WS_LOWER_TRIM_END,
    WS_LOWER_TRIM_COUNT,
};
*/

/* @debug not working, but not part of this branch's goal.
struct WsSections ws_section_lower_trim;
ws_section_lower_trim.machine_name = 'lower_trim';
*/

// I2C.
// @note Can be extended to 256 registers as needed, and presets can be set in the array.
volatile uint8_t i2c_registers[32] = {0x00};
#define I2C_ADDRESS 0x09

// Button.
// PC3.
#define BUTTON_TIMER_BASE 5
// @todo these debounce intervals need some tweaking.
#define BUTTON_ON_ATLEAST 20
#define BUTTON_OFF_ATLEAST 30
uint16_t button1_timer = BUTTON_TIMER_BASE;
uint8_t button1_state = 0; // @todo TBD button function.

#define COMET_DEFAULT_TIMER_BASE 250 // 1/4s
#define COMET_LENGTH 3
uint16_t comet_timer_base = COMET_DEFAULT_TIMER_BASE;

// @note: const does not work for these as pointers are pointed to them.
// Red comet.
static uint32_t comet_colors_0[3] = {
    0xC00000, 0x300000, 0x100000,
};

// Blue comet.
// @todo Blue compensation (for low voltage).
static uint32_t comet_colors_1[3] = {
    0x0000c0, 0x000030, 0x000010,
};

// @todo take gamma into account
// https://hackaday.com/2016/08/23/rgb-leds-how-to-master-gamma-and-hue-for-perfect-brightness/ .

// This complexity because of compilier directives; usaged also changes, ex:
// (*comet_colors_current)[position_within_comet]; or comet_colors_current = &comet_colors_1;
uint32_t (*comet_colors_current)[3] = &comet_colors_0;
int comet_selection = 0;

static int8_t comet_position = MIN_LED;
static int8_t comet_direction = 1; // or -1
volatile uint16_t comet_timer = COMET_DEFAULT_TIMER_BASE;
static volatile uint8_t comet_dirty = 1; // or 0

// Adapated from https://github.com/cnlohr/ch32v003fun/blob/master/examples/adc_fixed_fs/adc_fixed_fs.c .
// TIM1C1 uses PD2.
void init_timer() {
    // @todo change to use TIM2? Would also change ISR name.
    RCC->APB2PCENR |= RCC_APB2Periph_TIM1; // > APB2 peripheral clock enable register.
    // @todo why both TIM1 and TIM2?
    TIM1->CTLR1 |= TIM_CounterMode_Up | TIM_CKD_DIV1;
    TIM1->CTLR2 = TIM_MMS_1;
    
    // @debug now at 1/4s
    TIM1->ATRLR = SOTW_ATRLR-1; // > Auto-reload value register; the counter stops when ATRLR is empty.
    TIM1->PSC = SOTW_PSC-1; // > Counting clock prescaler.
    TIM1->RPTCR = 0; // > Recurring count value register.
    TIM1->SWEVGR = TIM_PSCReloadMode_Immediate; // > Event generation register.

    NVIC_EnableIRQ(TIM1_UP_IRQn); // (TIM1 Update Interrupt)
    TIM1->INTFR = ~TIM_FLAG_Update; // > Interrupt status register. 
    TIM1->DMAINTENR |= TIM_IT_Update; // > DMA/interrupt enable register.
    TIM1->CTLR1 |= TIM_CEN;
}

// Both adapted from https://github.com/cnlohr/ch32v003fun/blob/master/examples/adc_fixed_fs/adc_fixed_fs.c .
void TIM1_UP_IRQHandler(void) __attribute__((interrupt));
void TIM1_UP_IRQHandler() {
    timer_tick = 1;

    // @todo what does this do? from source code.
    if(TIM1->INTFR & TIM_FLAG_Update) {
        TIM1->INTFR = ~TIM_FLAG_Update;
    }
}

/*
    Callback that returns a color for a LED.
    Color fornat is 0xRRGGBB.
*/
uint32_t WS2812BLEDCallback( int ledno )
{
    // @todo this will have to have a switch to handle different animations.

    uint position_within_comet = comet_position - ledno;
    if(comet_direction < 0) position_within_comet = ledno - comet_position;

    if( position_within_comet < 0 ) return (uint32_t) 0x000000;

    if( position_within_comet > (COMET_LENGTH - 1)) return (uint32_t) 0x000000;

    return (*comet_colors_current)[position_within_comet];
}

void cometUpdateHandler() {
    comet_timer = comet_timer_base;

    comet_position += comet_direction;

    if( comet_position < MIN_LED ) {
        comet_position = MIN_LED;
        comet_direction = 1;
    }
    if( comet_position > LEDS ) {
        comet_position = LEDS - (COMET_LENGTH - 1);
        comet_direction = -1;
    }
    comet_dirty = 0;

    WS2812BDMAStart(LEDS);
}


void init_gpio() {
    // funGpioInitAll(); // @todo from i2c code... for using fun* functions.

    GPIO_port_enable(GPIO_port_C);

    // PC3 for button1.
    // @note changed to pullDown.    
    GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 3), GPIO_pinMode_I_pullDown, GPIO_Speed_In);

    // i2c_slave.
    funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
    funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL
    /* @debug will use lib instead
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

    Delay_Ms( 100 );

    DYN_GPIO_WRITE(GPIOC, CFGLR, (GPIO_CFGLR_t) { .PIN3 = GPIO_CFGLR_IN_FLOAT });

    GPIO_CFGLR_t ioc = DYN_GPIO_READ(GPIOC, CFGLR);
    */
    /* from source...
    ioc.PIN0 = GPIO_CFGLR_OUT_10Mhz_PP,
	ioc.PIN1 = GPIO_CFGLR_IN_ANALOG;
	DYN_GPIO_WRITE(GPIOC, CFGLR, ioc);
    */
    
    // @todo now what??? *********

    /* from AI....
    GPIOC->CRH &= ~GPIO_CRH_MODE3;
    GPIOC->CRH |= GPIO_CRH_MODE3_1;
    GPIOC->CRH |= GPIO_CRH_CNF3_0;
    GPIOC->CRH |= GPIO_CRH_CNF3_1;
    */
}

void onI2cWrite(uint8_t reg, uint8_t length) {
    // @todo
}

void onI2cRead(uint8_t reg) {
    // @todo
}

int main()
{
	SystemInit();
    // Let things settle.
    Delay_Ms( 100 );

    printf("In Main\r\n"); // @debug

    init_gpio();

    WS2812BDMAInit();

    init_timer();

    // Let things settle.
    Delay_Ms( 100 );

    // Address, registers, registers length, onWrite callback, onRead callback, read only.
    // > The I2C1 peripheral can also listen on a secondary address. [see Readme]
    SetupI2CSlave(I2C_ADDRESS, i2c_registers, sizeof(i2c_registers), onI2cWrite, onI2cRead, false);

    printf("In Main before while()\r\n"); // @debug

    while(1)
    {
        // @todo button(s) occasional polling and debounce here.
        if ( timer_tick ) {
            timer_tick = 0;

            // Comet, but @todo w/generic animation handlers.
            comet_timer--;
            if( comet_timer == 0 ) {
                cometUpdateHandler();
                // printf("comet handler\r\n"); @debug
            }

            button1_timer--;
            // @todo somehow this needs to be moved to a button handler.
            if( button1_timer == 0 ) {
                /*
                int test = GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_C, 3)); // @debug
                if ( test ) {
                    printf("button1 !!: %d\r\n", test); // @debug
                    //while (1) { } // debug.
                }
                */

                // Debounce button.
                // Adapted from https://stackoverflow.com/a/48435065 .
                if (button1_state > 1)
                {
                    button1_state -= 2;
                    // printf("button1_state >1 : %d\r\n", button1_state); // @debug
                }
                else
                // @note with pulldown, there's a short periodvat startup when button is high!
                if ( (!GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_C, 3))) != (!button1_state) ) {
                    printf("if != TRUE \r\n"); // @debug
                    if (button1_state)
                        button1_state = BUTTON_OFF_ATLEAST * 2 + 0;
                    else
                        button1_state = BUTTON_ON_ATLEAST * 2 + 1;
                }
                if ( button1_state & 1 ) {
                    printf("button1_state &1 TRUE : %d\r\n", button1_state); // @debug
                    button1_state = 0;

                    switch (comet_selection)    {
                        case 0: {
                            comet_colors_current = &comet_colors_1;
                            comet_selection = 1;
                            break;
                            }
                        case 1: {
                            comet_colors_current = &comet_colors_0;
                            comet_selection = 0;
                            break;
                            }
                    }
                    printf("comet_selection: %d\r\n\r\n", comet_selection); // @debug
                }
                // printf("button1_timer reset to BASE\r\n"); // @debug
                button1_timer = BUTTON_TIMER_BASE;
            }

        }
        
    }

}
