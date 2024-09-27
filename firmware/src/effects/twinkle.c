#include "effects/twinkle.h"

#include <rand.h>

#define Twinkle_Timer_offset 1 // 2 bytes
#define Twinkle_Frequency_offset 4

const uint8_t twinkle_defaults[5] = {
    EFFECT_TWINKLE,
    0, // Twinkle_Timer_default_H = 0;
    75, // Twinkle_Timer_default_L = 150; // @debug testing.
    1, // Twinkle_Mode_default - 1 = Random, 1+ TBD, make Enum?
    128, // Twinkle_Frequency_default, out of 255
};

uint8_t Twinkle_state = 0;

// @debug untested.

int effect_twinkle(Things_t thing, int flag) {
    uint8_t state;
    uint8_t dirty = 0;
    uint8_t r; // For random.

    // printf("In effect_twinkle, thing is: %u, flag is: %d\r\n", thing, flag); // @debug = 1 for Stars, from Things_t in global.h.

    if (flag == 1) {
        // Copy defaults to registry. 
        constToRegCopy(registry, reg_thing_start[thing], twinkle_defaults, 0, sizeof(twinkle_defaults) * sizeof(uint8_t));

        // @debug how about this?
        thing_timer[thing] = 10; // Come back around soon from the update loop.
        return 0; // Early!
    }
    else
    {
        for (int i = 0; i < STARS_GPIO_PINS_NUM; i++)
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
    }

    thing_timer[thing] = registry[reg_thing_start[thing] + Twinkle_Timer_offset] * 256 + registry[reg_thing_start[thing] + Twinkle_Timer_offset + 1];

    return dirty;
}