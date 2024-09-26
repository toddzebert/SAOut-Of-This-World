/*
 * main.c
 *
*/

/*
    *** THESE ARE FOR CH32V003F4P6 (TSSOP-20) Eval Board ***
    *** BUT SAO will use ...F4U6 (QFN-20) chip ***
    *** EACH have 20pins so pins should be mostly similar ***

    From ws2812b_dma_spi_led_driver.h:
        For the CH32V003 this means output will be on PORTC Pin 6
        void DMA1_Channel3_IRQHandler( void );

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
        // PC0 = 32 (@debug for now) - did not work! AKA TIM2CH3 (interrupt)
        // PD7 = 55 (@debug for now) - did not work! AKA NRST (reset/bootloader related), TIM2C4 (interrupt)  
        PD2 = 50 (@debug for now)
        PD3 = 51 (@debug for now)
        PD4 = 52 (@debug for now)
    
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


*/

#include "main.h"

// Stars GPIO pins.
// @debug PC0 (32), PD7 (55) did not work for unknown reasons.
const uint8_t stars_gpio_pins[STARS_GPIO_PINS_NUM] = { 50, 51, 52 };

volatile uint8_t timer_tick = 0;

// I2C.
#define I2C_ADDRESS 0x09

// Reserved registers. Meant to be R/O but the lib doesn't support it, so this is the backup copy.
#define REG_RESERVED_RO_LENGTH 2 // Up to 16 avail.

const uint8_t reg_reserved_ro[REG_RESERVED_RO_LENGTH] = {
    0, // API Major version,
    3, // API Minor version
    // @todo more?
};

// @todo take gamma into account
// https://hackaday.com/2016/08/23/rgb-leds-how-to-master-gamma-and-hue-for-perfect-brightness/ .


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
    // @todo why both TIM1CH1 and ...CH2?
    TIM1->CTLR1 |= TIM_CounterMode_Up | TIM_CKD_DIV1;
    TIM1->CTLR2 = TIM_MMS_1; // @debug CAN WE DO WITHOUT THIS?
    
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


// Init non-i2c GPIO.
void init_gpio() {
    // funGpioInitAll(); // This doesn't need to be called as the i2c lib does.
    // ... so place this below the i2c init.

    // Stars LEDs.
    for (int i = 0; i < STARS_GPIO_PINS_NUM; i++) {
        funPinMode(stars_gpio_pins[i] , GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );
    }
}


void starsUpdate()
{
    for (int i = 0; i < STARS_GPIO_PINS_NUM; i++) {
        // We'll let the compiler optimize this away.
        // @todo or maybe use: !!registry[REG_STARS_LED_START + i]
        if (registry[REG_STARS_LED_START + i])
        {
            funDigitalWrite( stars_gpio_pins[i], FUN_HIGH );
        }
        else {
            funDigitalWrite( stars_gpio_pins[i], FUN_LOW );
        }
    }   
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
    // Check protected "RO" registers and replace with our settings.
    // @debug untested.
    if (reg < REG_RESERVED_RO_LENGTH)
    {
        constToRegCopy(registry, 0, reg_reserved_ro, 0, sizeof(reg_reserved_ro) * sizeof(uint8_t));
    }
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

// @debug This is never called directly?
void init_i2c() {
    // i2c_slave.
    funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
    funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL

    // Address, registers, registers length, onWrite callback, onRead callback, read only.
    // > The I2C1 peripheral can also listen on a secondary address. [see Readme]
    SetupI2CSlave(I2C_ADDRESS, registry, sizeof(registry), onI2cWrite, onI2cRead, false);
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

    init_i2c();

    // Must be below init_i2c().
    init_gpio();

    // Let things settle.
    Delay_Ms( 200 );

    // funDigitalWrite( PC0, FUN_HIGH ); // @debug
    // Delay_Ms( 2000 ); // @debug

    // Init "things".
    button1Init();

    // @todo All the things inits should be done more dymamicly.
    eyesHandler(1);
    starsHandler(1);

    // WS2812 init and initial "start" to render. Must go after all "things" inits, ...Handler(1)'s.
    WS2812_Init();

    init_timer();

    // Let things settle.
    Delay_Ms( 100 );

    // Prep for main loop.
    int ws_dirty = 0;
    int stars_dirty = 0;

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
            eyes_timer--; // @todo alter once .c file converted to use arrays...
            if ( eyes_timer == 0 )
            {
                ws_dirty = eyesHandler(0) | ws_dirty;
            }

            // This should always be at the end, after all Things handlers.
            if (ws_dirty) {
                ws_dirty = 0;
                WS2812_Handler();
            }

            // Handle Stars.
            thing_timer[THING_STARS]--;
            if ( thing_timer[THING_STARS] == 0 )
            {
                stars_dirty = starsHandler(0) | stars_dirty; // The or is unnecessary, but for the sake of consistency...
            }

            // This could be a part of the Stars handler, but for the sake of consistency, it's here.
            if (stars_dirty) {
                stars_dirty = 0;
                starsUpdate(); // @todo.
            }
        }
    }
}
