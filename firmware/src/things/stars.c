#include "things/stars.h"

#include <ch32v003fun.h>

uint8_t stars_effect;

// Stars GPIO pins.
const uint8_t stars_gpio_h_pins[STARS_GPIO_H_PINS_NUM] = { PC0, PD0, PA2, PA1, PD3 };
const uint8_t stars_gpio_l_pins[STARS_GPIO_H_PINS_NUM] = { PD4, PD2 };

typedef struct {
    uint8_t know_state;
    uint8_t pwm_state;
    uint8_t counter;
    uint8_t threshold; // @todo may have to be uint16_t?

} Stars_State_t;

Stars_State_t Stars_PWM_State[STARS_GPIO_H_PINS_NUM];


int starsHandler(Event_t event)
{
    // printf("starsHandler\n"); // @debug
    if (!(event.thing == THING_STARS || event.thing == THING_ALL)) return 0;

    if (event.type == EVENT_INIT)
    {
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

void starsUpdate()
{
    /* @debug old, but working binary
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
    */
    for (int i = 0; i < STARS_GPIO_H_PINS_NUM; i++)
    {
        if (registry[REG_STARS_LED_START + i] == 0)
        {
            if (Stars_PWM_State[i].pwm_state == 0) continue; // Nothing to do.
            
            // Must be new 0 state.
            Stars_PWM_State[i].pwm_state = 0;
            funDigitalWrite(stars_gpio_h_pins[i], FUN_LOW);
        }
        else
        {
            if (Stars_PWM_State[i].know_state == registry[REG_STARS_LED_START + i])
            {
                Stars_PWM_State[i].counter++;

                if (Stars_PWM_State[i].counter >= Stars_PWM_State[i].threshold)
                {
                    Stars_PWM_State[i].counter = 0;
                    Stars_PWM_State[i].pwm_state = !Stars_PWM_State[i].pwm_state;

                    funDigitalWrite(stars_gpio_h_pins[i], Stars_PWM_State[i].pwm_state);
                }
            }
            else
            {
                // Change of known state.
                Stars_PWM_State[i].know_state = registry[REG_STARS_LED_START + i];
                Stars_PWM_State[i].counter = 0;
                Stars_PWM_State[i].pwm_state = 0;
                Stars_PWM_State[i].threshold = Stars_PWM_State[i].know_state; // @todo THIS IS A SWAG!
            }
        }
    }
}
