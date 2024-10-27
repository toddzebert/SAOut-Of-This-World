#include "things/upper_trim.h"

uint8_t upper_trim_effect;

int upperTrimHandler(int flag)
{
    // printf("upperTrimHandler\n"); // @debug

    upper_trim_effect = registry[reg_thing_start[THING_UPPER_TRIM]];
    if (!upper_trim_effect) upper_trim_effect = EFFECT_WS_COMET;

    switch (upper_trim_effect)
    {
    case EFFECT_RAW:
        // @todo return effect_raw(THING_UPPER_TRIM, event);
        break;

    case EFFECT_WS_COMET:
        return effect_ws_comet(THING_UPPER_TRIM, flag);
        break;
    
    default:
        // @todo what to do if given invalid effect?
        break;
    }

    return 0;
}