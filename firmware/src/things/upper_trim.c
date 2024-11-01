#include "things/upper_trim.h"

uint8_t upper_trim_effect;

int upperTrimHandler(Event_t event)
{
    // printf("upperTrimHandler\n"); // @debug
    if (!(event.thing == THING_UPPER_TRIM || event.thing == THING_ALL)) return 0;

    upper_trim_effect = registry[reg_thing_start[THING_UPPER_TRIM]];
    if (!upper_trim_effect) upper_trim_effect = EFFECT_WS_ROTATE; // @debug

    switch (upper_trim_effect)
    {
    case EFFECT_RAW:
        return effect_raw(THING_UPPER_TRIM, event);

    case EFFECT_WS_COMET:
        return effect_ws_comet(THING_UPPER_TRIM, event);

    case EFFECT_WS_ROTATE:
        return effect_ws_rotate(THING_UPPER_TRIM, event);
    
    default:
        // @todo what to do if given invalid effect?
        break;
    }

    return 0;
}
