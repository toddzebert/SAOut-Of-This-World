#include "effects/twinkle.h"

#include <rand.h>

#define Twinkle_Timer_offset 1 // 2 bytes
#define Twinkle_Mode_offset 3
#define Twinkle_Frequency_offset 4

const uint8_t twinkle_defaults[5] = {
    EFFECT_TWINKLE,
    0, // Twinkle_Timer_default_H = 0;
    75, // Twinkle_Timer_default_L = 150; // @debug testing.
    1, // Twinkle_Mode_default - 1 = Random, 1+ TBD, make Enum?
    128, // Twinkle_Frequency_default, out of 255
};

uint8_t Twinkle_state = 0;

int effect_twinkle_run(Things_t thing, Event_t event);

int effect_twinkle(Things_t thing, Event_t event)
{
    switch (event.type)
    {
    case EVENT_INIT:
        // Copy defaults to registry. 
        constToRegCopy(registry, reg_thing_start[thing], twinkle_defaults, 0, sizeof(twinkle_defaults) * sizeof(uint8_t));

        state_action[thing] = STATE_ACTION_ENTER;
        thing_timer[thing] = 10; // Come back around soon from the update loop.
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
    uint8_t state;
    uint8_t dirty = 0;
    uint8_t r; // For random.

    // @todo probably not needed. if (state_action[thing] == STATE_ACTION_ENTER) {}

    // printf("In effect_run_twinkle, thing is: %u, flag is: %d\r\n", thing, flag); // @debug = 1 for Stars, from Things_t in global.h.

    for (int i = 0; i < STARS_GPIO_H_PINS_NUM; i++)
    {
        state = (Twinkle_state >> i) & 1;
        r = (uint8_t) rnd_fun(0, 8);

        // printf("i: %d, state: %d, r: %d\n", i, state, r); // @debug

        if (r < registry[reg_thing_start[thing] + Twinkle_Frequency_offset])
        {
            // Flip it.
            state = !state;
            Twinkle_state ^= (1 << i);
            // Set reg LED.
            registry[reg_thing_led_start[thing] + i] = state;
            // Set dirty.
            dirty = 1;
        }
    }

    thing_timer[thing] = registry[reg_thing_start[thing] + Twinkle_Timer_offset] * 256 + registry[reg_thing_start[thing] + Twinkle_Timer_offset + 1];

    return dirty;
}