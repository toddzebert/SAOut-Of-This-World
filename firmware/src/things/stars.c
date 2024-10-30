#include "things/stars.h"

uint8_t stars_effect;

int starsHandler(Event_t event)
{
    // printf("starsHandler\n"); // @debug

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
