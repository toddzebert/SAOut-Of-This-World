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

/*
The I2C (inter-IC) bus can transfer data at different speeds, including: 
    Standard mode: 100 kbit/s 
    Fast mode: 400 kbit/s 
    Fast mode plus: 1 Mbit/s 
    High-speed mode: 3.4 Mbit/s 
    Ultra-fast mode: 5 Mbit/s 

WS2812B:
    Send data at speeds of 800Kbps.
*/

#include "main.h"

/*
#include <ch32v003fun.h>

#include <stdio.h>
#include <string.h> // for memset, more?
// #include <stdlib.h>
// #include <stdint.h>

#include <ch32v003_GPIO_branchless.h>
#include <i2c_slave.h>

#include "main.h"
#include "global.h"
#include "button.h"

#include "effects.h"
#include "things.h"
*/

#define LEDS 16
#define MIN_LED 0
// This is default in .h but making it explicit. > Has to be divisible by 4.

/* @todo registers:
    0-15: reserverd, 0 is version # - not writtable
    16-31: reserved for gloabal settings
    32-47: stars
    48-63: eyes
    64-95: upper trim
    96-127: lower trim
*/

// extern (see global.h)
// uint32_t ws_leds[16]; // @todo

volatile uint8_t timer_tick = 0;

// I2C.
#define I2C_ADDRESS 0x09

// @todo move all *comet* stuff below.
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


/**
 * @brief Initializes the timer for the 1/4s tick.
 *
 * The settings used are:
 * - TIM1 is enabled and used.
 * - The counter mode is Up-Counter.
 * - The clock division is 1.
 * - The master mode is set to Update.
 * - The Auto-Reload value is set to 250 (1/4s).
 * - The Prescaler is set to 48000.
 * - The Recurring Count Value is set to 0.
 * - The Event Generation is set to Immediate.
 * - The interrupt is enabled and the flag is cleared.
 * - The DMA/Interrupt enable register is set to Update.
 * - The Counter Enable bit is set.
 * 
 * Adapated from https://github.com/cnlohr/ch32v003fun/blob/master/examples/adc_fixed_fs/adc_fixed_fs.c .
 * TIM1C1 uses PD2.
 */
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
    /*
    uint position_within_comet = comet_position - ledno;
    if(comet_direction < 0) position_within_comet = ledno - comet_position;

    if( position_within_comet < 0 ) return (uint32_t) 0x000000;

    if( position_within_comet > (COMET_LENGTH - 1)) return (uint32_t) 0x000000;

    return (*comet_colors_current)[position_within_comet];
    */
    /* @todo remove
    if (ledno <= WS_EYES_LED_START ) {
        return eyesUpdate(ledno);
    }
    */

    return ws_leds[ledno];
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
    // @todo ?
}

// @note this only alerts on write, with values altered in registry.
void onI2cWrite(uint8_t reg, uint8_t length) {
    // @todo
}

// @note this only alerts on read, from values in registry.
void onI2cRead(uint8_t reg) {
    // @todo
}

void init_i2c() {
    // funGpioInitAll(); // @todo from i2c code... for using fun* functions.

    // i2c_slave.
    funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
    funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL

    // Address, registers, registers length, onWrite callback, onRead callback, read only.
    // > The I2C1 peripheral can also listen on a secondary address. [see Readme]
    SetupI2CSlave(I2C_ADDRESS, registry, sizeof(registry), onI2cWrite, onI2cRead, false);

    // Clear registry.
    // @todo ***** > warning: passing argument 1 of 'memset' discards 'volatile' qualifier from pointer target type [-Wdiscarded-qualifiers].
    memset((void *)registry, 0, sizeof registry);
}

int main()
{
	SystemInit();
    // Let things settle.
    Delay_Ms( 100 );

    printf("In Main\r\n"); // @debug

    init_gpio();

    // Init "things".
    button1Init();
    eyesInit();

    WS2812BDMAInit();

    init_timer();

    // Let things settle.
    Delay_Ms( 100 );



    printf("In Main before while()\r\n"); // @debug

    while(1)
    {
        // @todo button(s) occasional polling and debounce here.
        if ( timer_tick ) {
            timer_tick = 0;

            // Comet, but @todo w/generic animation handlers.
            /* @todo remove....
            comet_timer--;
            if( comet_timer == 0 ) {
                cometUpdateHandler();
            }
            */

            button1_timer--;
            if( button1_timer == 0 ) button1Handler();
        }
        
    }

}
