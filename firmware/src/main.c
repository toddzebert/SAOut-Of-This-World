/*
 * main.c
 *
*/

/*
    *** THESE ARE FOR CH32V003F4P6 (TSSOP-20) Eval Board ***
    *** BUT SAO will use ...F4U6 (QFN-20) chip ***
    *** EACH have 20pins so pins should be mostly similar ***
    CH32 = product line.
    V = QingKe RISC-V-based
    0 = QingKe V2 core (no FP)
    03 = General Purpose
    F = 20 pins
    4 = 16 Kbytes of Flash memory 
    P/U = TSSOP/QFN respectively
    6 = -40℃～85℃ (industrial-grade) 

    From ws2812b_dma_spi_led_driver.h:
        For the CH32V003 this means output will be on PORTC Pin 6 (MOSI)
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

    GLEDs:
        PC0
        PD0
        PA2

    Sense LEDs:
        PA1 (+), PD4 (-)
        PD3 (+), PD2 (-)
    
    For UART printf, on:
		CH32V003, Port PD5 (UTX, plus PD6/URX), 115200 8n1
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
const uint8_t stars_gpio_h_pins[STARS_GPIO_H_PINS_NUM] = { PC0, PD0, PA2, PA1, PD3 };
const uint8_t stars_gpio_l_pins[STARS_GPIO_H_PINS_NUM] = { PD4, PD2 };

// Timers.
volatile uint8_t timer_tick = 0;
volatile uint8_t timer_tick_count = 0;
volatile uint8_t timer_tock = 0;

// I2C.
#define I2C_ADDRESS 0x09

// Reserved registers. Meant to be R/O but the lib doesn't support it, so this is the backup copy.
#define REG_RESERVED_RO_LENGTH 2 // Up to 16 avail.

const uint8_t reg_reserved_ro[REG_RESERVED_RO_LENGTH] = {
    0, // API Major version,
    3, // API Minor version
    // @todo more?
};

void systick_init(void)
{
    // See https://github.com/cnlohr/ch32v003fun/blob/master/examples/systick_irq/systick_irq.c .
	// Reset any pre-existing configuration
	SysTick->CTLR = 0x0000;
	
	// Set the compare register to trigger once per millisecond
	SysTick->CMP = SYSTICK_ONE_MILLISECOND - 1;

	// Reset the Count Register.
	SysTick->CNT = 0x00000000;
	
	// Set the SysTick Configuration
	// NOTE: By not setting SYSTICK_CTLR_STRE, we maintain compatibility with
	// busywait delay funtions used by ch32v003_fun.
	SysTick->CTLR |= SYSTICK_CTLR_STE   |  // Enable Counter
	                 SYSTICK_CTLR_STIE  |  // Enable Interrupts
	                 SYSTICK_CTLR_STCLK ;  // Set Clock Source to HCLK/1
	
	// Enable the SysTick IRQ
	NVIC_EnableIRQ(SysTicK_IRQn);
}

// Ticks are 10ths of a millisecond. Tocks are 1ms.
void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
    timer_tick_count++; // 1's indexed.
    timer_tick = 1;

    if (timer_tick_count == 10)
    {
        timer_tick_count = 0;
        timer_tock = 1;
    }

    // Set the compare register to trigger once per 10th millisecond.
    SysTick->CMP += SYSTICK_ONE_TENTH_MILLISECOND; // @debug was SYSTICK_ONE_MILLISECOND;

	// Clear the trigger state for the next IRQ
	SysTick->SR = 0x00000000;
}

// Init non-i2c GPIO.
void init_gpio() {
    // funGpioInitAll(); // This doesn't need to be called as the i2c lib does.
    // ... so place this below the i2c init.

    // Stars LEDs.
    for (int i = 0; i < STARS_GPIO_H_PINS_NUM; i++)
    {
        funPinMode(stars_gpio_h_pins[i], GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );
    }
    
    // These are the Sense (star) LEDs.
    for (int i = 0; i < STARS_GPIO_L_PINS_NUM; i++)
    {
        funPinMode(stars_gpio_l_pins[i], GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );
        funDigitalWrite( stars_gpio_h_pins[i], FUN_LOW );
    }
}

// @todo move this to stars.c.
void starsUpdate()
{
    for (int i = 0; i < STARS_GPIO_H_PINS_NUM; i++) {
        // We'll let the compiler optimize this away.
        // @todo or maybe use: !!registry[REG_STARS_LED_START + i]
        if (registry[REG_STARS_LED_START + i])
        {
            funDigitalWrite( stars_gpio_h_pins[i], FUN_HIGH );
        }
        else
        {
            funDigitalWrite( stars_gpio_h_pins[i], FUN_LOW );
        }
    }   
}

void copyInRegReservedGlobal()
{
    // @todo future use?
}

/**
 * @brief Copy the "read-only" registry values from the reserved constants to the actual registry.
 *
 * While it's intended to be called once at startup, it'll be called again if the registry is changed.
 *
 * @note This function exists since the lib doesn't support R/O registers.
 */
void copyInRegReservedRO()
{
    constToRegCopy(registry, 0, reg_reserved_ro, 0, sizeof(reg_reserved_ro) * sizeof(uint8_t));
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
    if (reg < REG_RESERVED_RO_LENGTH) copyInRegReservedRO();
    // @todo issue reg event.
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

    Delay_Ms( 200 ); // Let things settle.

    copyInRegReservedRO();

    copyInRegReservedGlobal();

    // printNon0Reg(registry); // @debug

    // funDigitalWrite( PC0, FUN_HIGH ); // @debug
    Delay_Ms( 200 ); // Let things settle.

    // Init button1. @debug its broken since moving to fun*() usage.
    // @debug temp: button1Init();

    // Init "things".
    // @todo All the things inits should be done more dymamicly.
    Event_t event_init;
    event_init.type = EVENT_INIT;
    eyesHandler(event_init);
    starsHandler(event_init);
    upperTrimHandler(event_init);
    lowerTrimHandler(event_init);

    // WS2812 init and initial "start" to render. Must go after all "things" inits, ...Handler(1)'s.
    WS2812_Init();

    // @debug was used for dev, but not anymore (maybe). init_timer();

    // Let things settle.
    Delay_Ms( 200 );

    // Prep for main loop.
    int ws_dirty = 0;
    int stars_dirty = 0;

    Event_t event_run;
    event_run.type = EVENT_RUN;

    systick_init();

    // printf("In main, thing_timer[THING_EYES]: %d\n", thing_timer[THING_EYES]); // @debug
    printf("In main, right before loop.\n"); // @debug

    while(1)
    {
        // @todo button(s) occasional polling and debounce here, and create event.

        if (timer_tick)
        {
            timer_tick = 0;

            // @todo do what we want here.
        }

        if (timer_tock)
        {
            timer_tock = 0;
            // Handle button1.
            button1_timer--;
            if ( button1_timer == 0 ) button1Handler();

            // Handle Eyes.
            thing_tock_timer[THING_EYES]--;
            if ( thing_tock_timer[THING_EYES] == 0 )
            {
                // printf("In main, thing_timer[THING_EYES] == 0.\n"); // @debug
                ws_dirty = eyesHandler(event_run) || ws_dirty;
            }

            // Handle Upper Trim.
            thing_tock_timer[THING_UPPER_TRIM]--;
            if ( thing_tock_timer[THING_UPPER_TRIM] == 0 )
            {
                ws_dirty = upperTrimHandler(event_run) || ws_dirty;
            }

            // Handle Lower Trim.
            thing_tock_timer[THING_LOWER_TRIM]--;
            if ( thing_tock_timer[THING_LOWER_TRIM] == 0 )
            {
                ws_dirty = lowerTrimHandler(event_run) || ws_dirty;
            }

            // This should always be at the end, after all WS Things handlers.
            if (ws_dirty) {
                ws_dirty = 0;
                WS2812_Handler();
            }

            // Handle Stars (not a WS Thing).
            thing_tock_timer[THING_STARS]--;
            if ( thing_tock_timer[THING_STARS] == 0 )
            {
                stars_dirty = starsHandler(event_run) || stars_dirty; // The || is unnecessary, but for the sake of consistency...
            }

            // This could be a part of the Stars handler, but for the sake of consistency, it's here.
            if (stars_dirty) {
                stars_dirty = 0;
                starsUpdate(); // @todo ?
            }
        }
    }
}
