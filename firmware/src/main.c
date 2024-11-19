/*
 * main.c
 *
*/

#include <rand.h> // @debug

/*
    From ws2812b_dma_spi_led_driver.h:
        void DMA1_Channel3_IRQHandler( void );

    Timer1 is used for Stars PWM.
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

// Timers.
volatile uint8_t timer_tock = 0;

// I2C.
#define I2C_ADDRESS 0x09

// Reserved registers. Meant to be R/O but the lib doesn't support it, so this is the backup copy.
#define REG_RESERVED_RO_LENGTH 2 // Up to 16 avail.

const uint8_t reg_reserved_ro[REG_RESERVED_RO_LENGTH] = {
    0, // API Major version,
    3, // API Minor version
};

struct {
    uint8_t Brightness_mode: 1;
} Global_state = {
    .Brightness_mode = 0
};

#define BRIGHTNESS_MODE_DISABLED 0
#define BRIGHTNESS_MODE_ENABLED 1

#define GLOBAL_RETURN_NONE 0
#define GLOBAL_RETURN_STOP_PROP 1

/**
 * @brief Initializes the SysTick counter to fire once per millisecond.
 *
 * Resets any pre-existing configuration, sets the compare register to trigger
 * once per millisecond, resets the Count Register, sets the SysTick
 * configuration, and enables the SysTick IRQ.
 *
 * @note This is a modified version of the example
 *       <https://github.com/cnlohr/ch32v003fun/blob/master/examples/systick_irq/systick_irq.c>.
 */
void systick_init(void)
{
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
	                 SYSTICK_CTLR_STCLK |  // Set Clock Source to HCLK/1
                     SYSTICK_CTLR_STRE;    // Enable Auto Reload
    
	
	// Enable the SysTick IRQ
	NVIC_SetPriority(SysTicK_IRQn, 2 << 6);
    NVIC_EnableIRQ(SysTicK_IRQn);
}


void SysTick_Handler(void) __attribute__((interrupt));
/**
 * @brief Interrupt Service Routine for SysTick timer.
 *
 * This function handles the SysTick interrupt, which occurs every 1ms. 
 * It sets the `timer_tock` flag to indicate the timer tick has occurred
 * and clears the SysTick status register to reset the interrupt trigger state.
 * 
 * Tocks are 1ms.
 */
void SysTick_Handler(void)
{
    timer_tock = 1;

	// Clear the trigger state for the next IRQ
	SysTick->SR = 0x00000000;
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
    // printf("onI2cWrite(%d, %d)\n", reg, length); // @debug

    // Check protected "RO" registers and replace with our settings.
    // @debug untested.
    if (reg < REG_RESERVED_RO_LENGTH) copyInRegReservedRO();

    Event_t reg_change_event = {
        .type = EVENT_REG_CHANGE,
        .thing = THING_ALL,
        .data.reg_change.reg = reg,
        .data.reg_change.length = length
    };

    eventPush(reg_change_event);
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


/**
 * @brief Initializes the I2C slave interface.
 *
 * Initializes the I2C slave interface pins (SDA and SCL) and sets up the I2C slave
 * using the i2c_slave library. The I2C address is set to I2C_ADDRESS, the registry
 * is set to the registry array of length sizeof(registry), and the onI2cWrite and
 * onI2cRead callback functions are set.
 *
 * @note This function is intended to be called once, at initialization time.
 */
void init_i2c() {
    // i2c_slave.
    funPinMode(PC1, GPIO_CFGLR_OUT_2Mhz_AF_OD); // SDA // @debug was GPIO_CFGLR_OUT_10Mhz_AF_OD.
    funPinMode(PC2, GPIO_CFGLR_OUT_2Mhz_AF_OD); // SCL // @debug was GPIO_CFGLR_OUT_10Mhz_AF_OD.

    SetupI2CSlave(I2C_ADDRESS, registry, sizeof(registry), onI2cWrite, onI2cRead, false);
}


/**
 * @brief The global event handler.
 *
 * This function is responsible for handling Global events that are not specific to
 * a specific Things. If the event is a button press, it will handle it if the global
 * brightness mode is enabled. If the brightness mode is disabled, it will
 * ignore the event.
 *
 * @param[in] event The event to handle.
 * @return The return value is one of the GLOBAL_RETURN_* constants. If
 * GLOBAL_RETURN_STOP_PROP is returned, the event will not be propagated to
 * other handlers. If GLOBAL_RETURN_NONE is returned, the event will be
 * propagated to other handlers.
 */
int globalHandler(Event_t event)
{
    switch (event.type)
    {
        case EVENT_BUTTON:
            if (event.data.button.type == BUTTON_LONG_PRESSED)
            {
                // Check if correct button.
                if (event.data.button.num == 1)
                {
                    if (Global_state.Brightness_mode == BRIGHTNESS_MODE_ENABLED)
                        return GLOBAL_RETURN_STOP_PROP;
                    else
                        return GLOBAL_RETURN_NONE;
                }
                else
                {
                    if (Global_state.Brightness_mode == BRIGHTNESS_MODE_ENABLED)
                    {
                        Global_state.Brightness_mode = BRIGHTNESS_MODE_DISABLED;
                        return GLOBAL_RETURN_NONE;
                    }
                    else
                    {
                        Global_state.Brightness_mode = BRIGHTNESS_MODE_ENABLED;
                        return GLOBAL_RETURN_STOP_PROP;
                    }
                    
                }
            }
            else if (event.data.button.type == BUTTON_PRESSED)
            {
                if (Global_state.Brightness_mode == BRIGHTNESS_MODE_DISABLED)
                    return GLOBAL_RETURN_NONE;

                // OK, time to change brightness.
                if (event.data.button.num == BUTTON_LEFT)
                {
                    if (registry[REG_GLOBAL_BRIGHTNESS] > 0)
                        registry[REG_GLOBAL_BRIGHTNESS]--;

                    return GLOBAL_RETURN_STOP_PROP;
                }
                else if (event.data.button.num == BUTTON_RIGHT)
                {
                    if (registry[REG_GLOBAL_BRIGHTNESS] < 8)
                        registry[REG_GLOBAL_BRIGHTNESS]++;

                    return GLOBAL_RETURN_STOP_PROP;
                }
            }
            
            break;

        default:
            break;
    }

    return GLOBAL_RETURN_NONE;
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

    init_i2c();

    copyInRegReservedRO();

    copyInRegReservedGlobal();

    funGpioInitAll();
    // @debug testing this.  From PA's issue.... probably no longer needed.
    RCC->APB2PCENR |= RCC_AFIOEN;
    //Delay_Ms(1);
    AFIO->PCFR1 &= ~AFIO_PCFR1_PA12_REMAP;

    // Init "things".
    const Event_t Event_Init = {
        .type = EVENT_INIT,
        .thing = THING_ALL
    };

     // @todo All the things inits should be done more dynamically.
    buttonHandler(Event_Init);
    eyesHandler(Event_Init);
    starsHandler(Event_Init);
    upperTrimHandler(Event_Init);
    lowerTrimHandler(Event_Init);

    // WS2812 init and initial "start" to render. Must go after all "things" inits, ...Handler(1)'s.
    WS2812_Init();

    // Prep for main loop.
    int ws_dirty = 0;

    const Event_t Event_Run = {
        .type = EVENT_RUN,
        .thing = THING_ALL
    };

    Delay_Ms(500); // @debug

    // printf("in main, size of Event_t: %d\n", sizeof(Event_t)); // @debug

    // for (int i = 0; i < 10; i++) printf("rand 0|1: %d\n", rnd_fun(0, 1) & 0x01); // @debug

    systick_init();

    printf("In main, right before loop.\n"); // @debug

    while(1)
    {
        if (timer_tock)
        {
            timer_tock = 0;

            // Use of this should be limited as its expensive.
            while (!eventQueueEmpty())
            {
                // printf("in main event while loop.\n"); // @debug
                Event_t event = eventPop();
                printf("In main loop event while - type, thing: %d %d\n", event.type, event.thing); // @debug

                // Global gets priority, and return here is not dirty, but a return state.
                if (globalHandler(event) == GLOBAL_RETURN_STOP_PROP) continue;

                buttonHandler(event); // @todo can we think of a reason for this?

                ws_dirty = eyesHandler(event) || ws_dirty;
                ws_dirty = starsHandler(event) || ws_dirty;
                ws_dirty = upperTrimHandler(event) || ws_dirty;
                ws_dirty = lowerTrimHandler(event) || ws_dirty;
            }

            // Handle buttons.
            thing_tock_timer[THING_BUTTONS]--;
            if ( thing_tock_timer[THING_BUTTONS] == 0 ) {
                // @todo Also, perhaps this should go higher in the loop so the event can be processed
                // as part of the existing global event handling conditional.
                buttonHandler(Event_Run);
            }

            // Handle Eyes.
            thing_tock_timer[THING_EYES]--;
            if ( thing_tock_timer[THING_EYES] == 0 )
            {
                ws_dirty = eyesHandler(Event_Run) || ws_dirty;
            }

            // Handle Upper Trim.
            thing_tock_timer[THING_UPPER_TRIM]--;
            if ( thing_tock_timer[THING_UPPER_TRIM] == 0 )
            {
                ws_dirty = upperTrimHandler(Event_Run) || ws_dirty;
            }

            // Handle Lower Trim.
            thing_tock_timer[THING_LOWER_TRIM]--;
            if ( thing_tock_timer[THING_LOWER_TRIM] == 0 )
            {
                ws_dirty = lowerTrimHandler(Event_Run) || ws_dirty;
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
                starsHandler(Event_Run);
                // @note Stars updates (like dirty state) are handled by the PWM ISR.
            }
        }
    }

    __WFI(); // Wait for Interrupt.
}
