#include "effects/twinkle.h"

#include <rand.h>

#define Twinkle_Timer_offset 1 // 2 bytes
#define Twinkle_Mode_offset 3
#define Twinkle_Frequency_offset 4

const uint8_t twinkle_defaults[5] = {
    EFFECT_TWINKLE,
    0, // Twinkle_Timer_default_H = 0;
    10, // Twinkle_Timer_default_L = 150; // @debug testing, was 25
    1, // Twinkle_Mode_default - 1 = Random, 1+ TBD, make Enum?
    3, // Twinkle_Frequency_default, out of 255. // @debug testing, was 8.
};

// State. These are the target vals.
uint8_t Twinkle_state[THING_COUNT];

int effect_twinkle_run(Things_t thing, Event_t event);

int effect_twinkle(Things_t thing, Event_t event)
{
    switch (event.type)
    {
    case EVENT_INIT:
        // Copy defaults to registry. 
        constToRegCopy(registry, reg_thing_start[thing], twinkle_defaults, 0, sizeof(twinkle_defaults) * sizeof(uint8_t));

        // Set initial target values.
        for (int i = 0; i < STARS_GPIO_H_PINS_NUM; i++)
        {
            // 63 + (max) 127 (after > 1) + (max) 63 (after > 2) + 2 = 255.
            Twinkle_state[i] = 63 + ((uint8_t) rnd_fun(0, 8) >> 1) + ((uint8_t) rnd_fun(0, 8) >> 2) + 2;
        }

        state_action[thing] = STATE_ACTION_ENTER;
        thing_tock_timer[thing] = 10; // Come back around soon from the update loop.

        return 0;

    case EVENT_RUN:
        return effect_twinkle_run(thing, event);

    case EVENT_REG_CHANGE:
        // @todo
        return 0;

    default:
        return 0;
    }
}


int effect_twinkle_run(Things_t thing, Event_t event)
{
    uint8_t dirty = 0;
    uint8_t target = 0;
    int new_val;
    uint8_t r; // For random.

    for (int i = 0; i < STARS_GPIO_H_PINS_NUM; i++)
    {
        target = REG_STARS_LED_START + i;
        // If not equal, reg val needs to move towards target val.
        if (Twinkle_state[i] != registry[target])
        {
            // Are we going up or down?
            if (Twinkle_state[i] > registry[target])
            {
                // Going up, by 1/4 the difference. The +1 is for rounding.
                new_val = registry[target] + ((Twinkle_state[i] - registry[target]) >> 2) + 1;

                // Avoid overrun.
                if (new_val > Twinkle_state[i])
                    new_val = Twinkle_state[i];
            }
            else
            {
                // Going down, by 1/4 the difference. The -1 is for rounding.
                new_val = registry[target] - ((registry[target] - Twinkle_state[i]) >> 2) - 1;

                // Avoid under-run.
                if (new_val < Twinkle_state[i])
                    new_val = Twinkle_state[i];
            }

            registry[target] = (uint8_t) new_val;

            dirty = 1;
        }
        else
        {
            // Target reached, should we do something new?
            r = (uint8_t) rnd_fun(0, 8);  // 0-255.

            if ((uint8_t) rnd_fun(0, 8) < registry[reg_thing_start[thing] + Twinkle_Frequency_offset])
            {
                // Time to change. 63 Base is to avoid the "dead zone" - see gamma8 array.
                Twinkle_state[i] = registry[target]
                    ? 0 : 63 + ((uint8_t) rnd_fun(0, 8) >> 1) + ((uint8_t) rnd_fun(0, 8) >> 2) + 2;
            }
        }
    }

    thing_tock_timer[thing] = registry[reg_thing_start[thing] + Twinkle_Timer_offset] * 256 + registry[reg_thing_start[thing] + Twinkle_Timer_offset + 1];

    return dirty;
}
