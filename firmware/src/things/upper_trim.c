#include "things/upper_trim.h"

uint8_t upper_trim_effect;

int upperTrimHandler(Event_t event)
{
    // @debug below
    if (event.type > 7) printf("ERROR upperTrimHandler, event.type OOR: %d\r\n", event.type);
    if (event.type > 100) printf("ERROR upperTrimHandler, event.type OOR: %d\r\n", event.type);
    // @debug above

    // printf("upperTrimHandler\n"); // @debug
    if (!(event.thing == THING_UPPER_TRIM || event.thing == THING_ALL)) return 0;

    upper_trim_effect = registry[reg_thing_start[THING_UPPER_TRIM]];
    if (!upper_trim_effect) upper_trim_effect = EFFECT_WS_ROTATE;
    // @todo for when comet doesn't suck -(rnd_fun(0, 1) & 0x01) ? EFFECT_WS_ROTATE : EFFECT_WS_COMET;

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
