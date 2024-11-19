#include "things/stars.h"

#include <ch32v003fun.h>

// A round-about way of setting PWM_COUNT to 64.
#define PWM_BIT_DEPTH 6
#define PWM_COUNT (1 << PWM_BIT_DEPTH)

uint8_t stars_effect;

struct {
    GPIO_TypeDef *gpio;
    uint16_t pin;
    uint8_t idx;
    uint8_t fun_port;
} const Stars_Pin_info[STARS_COUNT] =
{
    {GPIOC, GPIO_Pin_0, 0, PC0},
    {GPIOD, GPIO_Pin_0, 0, PD0},
    {GPIOA, GPIO_Pin_2, 2, PA2},
    {GPIOA, GPIO_Pin_1, 1, PA1},
    {GPIOD, GPIO_Pin_3, 3, PD3}
};

// PD2 is also analog.
const uint8_t stars_gpio_l_pins[STARS_GPIO_H_PINS_NUM] = {PD4, PD2};

// For the ISR. A static counter var for PWM.
static int8_t idx = 0;

/**
 * @brief Handler for the stars thing.
 *
 * This function is a handler for the stars thing. It reads the effect ID from
 * the registry and calls the appropriate effect function, passing the thing ID
 * and the flag as arguments. During Init it sets up the GPIOs and timers.
 *
 * @param event The event struct containing the event type and thing ID.
 *
 * @return The return value is the LED "dirty" flag, which is meaningless for Stars.
 */
int starsHandler(Event_t event)
{
    if (!(event.thing == THING_STARS || event.thing == THING_ALL)) return 0;

    if (event.type == EVENT_INIT)
    {
        // Configure the GPIOs for soft PWM, all high side.
        for (uint8_t i = 0; i < STARS_COUNT; i++)
        {
            // CFGLR = port configuration register low.
            // Each pin is 4 bits (CNF & MODE), thus 4 * idx.
            // The first line resets the bits,
            // the second line sets them to output mode, 2MHz.
            // 00: Universal push-pull output mode, is the default setting (after reset).
            Stars_Pin_info[i].gpio->CFGLR &= ~(0xf << (4 * Stars_Pin_info[i].idx));
            Stars_Pin_info[i].gpio->CFGLR |=  (GPIO_Speed_2MHz << (4 * Stars_Pin_info[i].idx));
        }

        // Enable GPIOD and TIM2
        RCC->APB1PCENR |= RCC_APB1Periph_TIM2;
        
        // SMCFGR: default clk input is CK_INT
        TIM2->PSC = 0x0000; // Set TIM2 clock prescaler divider.
        // Set PWM total cycle width.
        // Automatic Reload Register (ATRLR) reloads the initial value for the CNT at the end of each count cycle.
        TIM2->ATRLR = 2400 - 1;
        TIM2->CNT = 0; // Reset counter.

        // Enable interrupt.
        TIM2->DMAINTENR |= TIM_UIE; // DMA/interrupt enable register; Timer Update interrupt enable.
        // Nested vector interrupt control (NVIC).
        NVIC_SetPriority(TIM2_IRQn, 0 << 6);
        NVIC_EnableIRQ(TIM2_IRQn);

        // Enable TIM2.
        TIM2->CTLR1 |= TIM_CEN; // Control register 1; Time counter enable.

        // These are the Sense (star) LEDs, low side.
        for (int i = 0; i < STARS_GPIO_L_PINS_NUM; i++)
        {
            funPinMode(stars_gpio_l_pins[i], GPIO_Speed_2MHz | GPIO_CNF_OUT_PP);
            funDigitalWrite(Stars_Pin_info[i].fun_port, FUN_LOW);
        }
    }
    
    // Get the stars effect, or default to twinkle.
    int stars_effect = registry[reg_thing_start[THING_STARS]];
    if (!stars_effect) stars_effect = EFFECT_TWINKLE;

    switch (stars_effect)
    {
        case EFFECT_RAW:
            return effect_raw(THING_STARS, event);

        case EFFECT_TWINKLE:
            return effect_twinkle(THING_STARS, event);
        
        default:
            // @todo what to do if given invalid effect?
            break;
    }

    return 0;
}

void TIM2_IRQHandler(void) __attribute__((interrupt));
void TIM2_IRQHandler(void)
{
    int8_t i;
    uint8_t g; // The resulting PWM value for a given star.
    
    idx--;

    if (!idx) idx = PWM_COUNT;

    for (i = 0; i < STARS_COUNT; i++)
    {
        // Get the PWM value, do brightness, and then LSR by 2 to reduces PWM range to 0-63 to keep it performant.
        g = registry[REG_STARS_LED_START + i];
        g = brightnessControl(g);
        g = gamma8[g] >> 2;

        if (idx == PWM_COUNT)
            Stars_Pin_info[i].gpio->BCR = Stars_Pin_info[i].pin; // BCR = Port bit reset.

        if (idx == g)
            Stars_Pin_info[i].gpio->BSHR = Stars_Pin_info[i].pin; // BSHR = Port bit set/reset register.
    }

    TIM2->INTFR &= ~TIM_UIF; // Interrupt Status Register; Update interrupt flag.
}


/* @note for future use.
void softpwm_deinit(void)
{
    TIM2->CTLR1 &= ~TIM_CEN;
}
*/
