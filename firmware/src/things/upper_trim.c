#include "things/upper_trim.h"

uint8_t upper_trim_effect;

int upperTrimHandler(Event_t event)
{
    // printf("upperTrimHandler\n"); // @debug
    if (!(event.thing == THING_UPPER_TRIM || event.thing == THING_ALL)) return 0;

    upper_trim_effect = registry[reg_thing_start[THING_UPPER_TRIM]];
    if (!upper_trim_effect) upper_trim_effect = EFFECT_WS_ROTATE; // @debug

    // Respond to presses from button 0 (the left one).
    if
    (
        event.type == EVENT_BUTTON &&
        event.data.button.num == 0 &&
        event.data.button.type == BUTTON_PRESSED
    )
    {
        // Rotate through the effects.
        uint8_t next_trim_effect = upper_trim_effect;
        switch (upper_trim_effect)
        {
            case EFFECT_RAW:
                next_trim_effect = EFFECT_WS_ROTATE;
                break;
            case EFFECT_WS_COMET:
                next_trim_effect = EFFECT_RAW;
                break;
            case EFFECT_WS_ROTATE:
                next_trim_effect = EFFECT_WS_COMET;
                break;
            default:
                next_trim_effect = EFFECT_WS_ROTATE;
                break;
        }
        registry[reg_thing_start[THING_UPPER_TRIM]] = next_trim_effect;
        Event_t Upper_Trim_Reinit = {
            .type = EVENT_INIT,
            .thing = THING_UPPER_TRIM
        };
        eventPush(Upper_Trim_Reinit);
        //printf("upper_trim.c: Changed to %d effect\n", next_trim_effect);
        return 0;
    }

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
