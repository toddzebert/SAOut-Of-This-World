/*
 * main.c
 *
*/

/*
    From ws2812b_dma_spi_led_driver.h:
        For the CH32V003 this means output will be on PORTC Pin 6

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

#define LEDS 16
#define MIN_LED 0
// This is default in .h but making it explicit. > Has to be divisible by 4.

volatile uint8_t timer_tick = 0;

// I2C.
#define I2C_ADDRESS 0x09

// @todo move all *comet* stuff below.
#define COMET_DEFAULT_TIMER_BASE 250 // 1/4s
#define COMET_LENGTH 3
uint16_t comet_timer_base = COMET_DEFAULT_TIMER_BASE;

// @note: const does not work for these as pointers are pointed to them.
// Red comet.
// @todo remove.
static uint32_t comet_colors_0[3] = {
    0xC00000, 0x300000, 0x100000,
};

// Blue comet.
// @todo Blue compensation (for low voltage).
// @todo remove.
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
 * @brief Initializes the timer for the 1ms tick.
 *
 * The settings used are:
 * - TIM1 is enabled and used.
 * - The counter mode is Up-Counter.
 * - The clock division is 1.
 * - The master mode is set to Update.
 * - The Auto-Reload value is set to 10 (1ms).
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
/**
 * @brief Interrupt handler for the TIM1 Update Interrupt.
 *
 * This function is called whenever the TIM1 counter has reached its
 * Auto-Reload value and the counter has been reloaded with the value
 * of the Auto-Reload register.
 *
 * The interrupt flag is reset by writing the opposite value of the flag
 * into the Interrupt Status Register.
 */
void TIM1_UP_IRQHandler() {
    timer_tick = 1;

    // Reset the timer interrupt flag.
    if(TIM1->INTFR & TIM_FLAG_Update) {
        TIM1->INTFR = ~TIM_FLAG_Update;
    }
}


/**
 * @brief Callback that returns a color for a LED.
 *
 * This function is called by the WS2812B LED driver library for each LED in the
 * strip. The function should return a uint32_t containing the color for the given
 * LED number. The color format is 0xRRGGBB.
 *
 * The function is called with the LED number as an argument. The LED number is
 * zero-based, meaning the first LED is number 0.
 *
 * The function should return the color for the given LED number. The color
 * should be in the format 0xRRGGBB.
 *
 * @param ledno The LED number for which the color should be returned.
 * @return The color for the given LED number.
 */
uint32_t WS2812BLEDCallback( int ledno )
{
    // @todo remove once code moved to comet.c.
    /*
    uint position_within_comet = comet_position - ledno;
    if(comet_direction < 0) position_within_comet = ledno - comet_position;

    if( position_within_comet < 0 ) return (uint32_t) 0x000000;

    if( position_within_comet > (COMET_LENGTH - 1)) return (uint32_t) 0x000000;

    return (*comet_colors_current)[position_within_comet];
    */

    uint32_t color;

    int index = 0;
    // char *thing = ""; // @debug

    if (ledno < EYES_COUNT) {
        index = REG_EYES_LED_START + (ledno * 3);
        // thing = "eyes"; // @debug
    }
    else if (ledno < (EYES_COUNT + UPPER_TRIM_COUNT)) {
        index = REG_UPPER_TRIM_LED_START + (ledno - EYES_COUNT) * 3;
        // thing = "upper trim"; // @debug
    }
    else
    {
        index = REG_LOWER_TRIM_LED_START + (ledno - EYES_COUNT - UPPER_TRIM_COUNT) * 3;
        // thing = "lower trim"; // @debug
    }

    // The WS2812B lib expects the color in the format 0x00RRGGBB.
    // @debug could also try casting the 3 consecutive bytes into a uint32_t (starting from index-1, and then clearing MSB).
    color = (uint32_t) registry[index] << 16 | (uint32_t) registry[index + 1] << 8 | (uint32_t) registry[index + 2];

    // @todo adjust for blue vs other colors.
    // @todo gamma adjustment.

    // printf("WS2812BLEDCallback: ledno %d thing %s reg index %d raw_color 0x%08lx\n", ledno, thing, index, raw_color); // @debug
    
    return color;
}

// @todo remove once code moved to comet.c.
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


/**
 * @brief Called when a write is received over I2C, with values altered in registry.
 * 
 * @param[in] reg The register address written to.
 * @param[in] length The number of bytes written.
 * 
 * @note This is a callback function and is not intended to be called directly.
 * @todo Check protected "RO" registers and replace.
 */
void onI2cWrite(uint8_t reg, uint8_t length) {
    // @todo check protected "RO" registers and replace.
}


/**
 * @brief Called when a read is requested over I2C from the registry, but is only an "alert".
 * 
 * @param[in] reg The register address read from.
 * 
 * @note This is a callback function and is not intended to be called directly.
 * @todo Check if we need to do anything here.
 */
void onI2cRead(uint8_t reg) {
    // @todo ?
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
    // @note the (void *) is a hack to try to avoid the warning.
    // @todo is this even needed? ******
    // memset((void *)registry, 0, sizeof registry);
}

/**
 * @brief Main entry point.
 * 
 * @details
 * This is the main entry point for the program. It initializes all the
 * peripherals and starts the main loop.
 * 
 * The main loop is an infinite loop that checks for button presses and
 * updates the LEDs accordingly. It also handles the WS2812B LEDs and makes
 * sure that the LEDs are updated when needed.
 * 
 * @note This function does not return.
 */
int main()
{
	SystemInit();
    // Let things settle.
    Delay_Ms( 200 );

    // printf("In Main\r\n"); // @debug

    init_gpio();

    // Let things settle.
    Delay_Ms( 200 );

    // Init "things".
    button1Init();

    // @todo All the things inits should be done more dymamicly.
    eyesInit();

    // WS2812 init and initial "start" to render. Must go after all "things" inits.
    WS2812BDMAInit();
    WS2812BDMAStart(LEDS);

    init_timer();

    // Let things settle.
    Delay_Ms( 100 );

    // printf("In Main before while()\r\n"); // @debug, but this is the first showing in monitor log.

    int ws_dirty = 0;

    while(1)
    {
        // @todo button(s) occasional polling and debounce here.
        // @todo perhaps make dirty more descriptive and make volatile global so updates only set if there's a change?
        // @todo ... or thing handlers should return a dirty value?
        if (timer_tick) {
            timer_tick = 0;

            // Handle button1.
            button1_timer--;
            if ( button1_timer == 0 ) button1Handler();

            // Handle Eyes.
            eyes_timer--;
            if ( eyes_timer == 0 )
            {
                eyesHandler();
                ws_dirty = 1;
            }

            // @todo have to handle GPIO stars diff from WS things!

            // This should always be at the end, after all Things handlers.
            if (ws_dirty) {
                ws_dirty = 0;
                WS2812BDMAStart(LEDS);
            }
        }
    }
}
