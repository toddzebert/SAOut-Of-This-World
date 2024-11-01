#include "things/lower_trim.h"

uint8_t lower_trim_effect;

int lowerTrimHandler(Event_t event)
{
    // printf("lowerTrimHandler\n"); // @debug
    if (!(event.thing == THING_LOWER_TRIM || event.thing == THING_ALL)) return 0;

    lower_trim_effect = registry[reg_thing_start[THING_LOWER_TRIM]];
    if (!lower_trim_effect) lower_trim_effect = EFFECT_WS_ROTATE;

    switch (lower_trim_effect)
    {
    case EFFECT_RAW:
        return effect_raw(THING_LOWER_TRIM, event);

    case EFFECT_WS_COMET:
        return effect_ws_comet(THING_LOWER_TRIM, event);

    case EFFECT_WS_ROTATE:
        return effect_ws_rotate(THING_LOWER_TRIM, event);
    
    default:
        // @todo what to do if given invalid effect?
        break;
    }

    return 0;
}
