#include "things/stars.h"

uint8_t stars_effect;

int starsHandler(int flag)
{
    // printf("starsHandler\n"); // @debug

    stars_effect = registry[reg_thing_start[THING_STARS]] || EFFECT_TWINKLE;

    switch (stars_effect)
    {
    case EFFECT_RAW:
        return effect_raw(THING_STARS, flag);
        break;

    case EFFECT_TWINKLE:
        return effect_twinkle(THING_STARS, flag);
        break;
    
    default:
        // @todo what to do if given invalid effect?
        break;
    }

    return 0;
}
