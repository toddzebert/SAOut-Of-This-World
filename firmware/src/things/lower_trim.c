#include "things/lower_trim.h"

uint8_t lower_trim_effect;

int lowerTrimHandler(Event_t event)
{
    // printf("lowerTrimHandler\n"); // @debug
    if (!(event.thing == THING_LOWER_TRIM || event.thing == THING_ALL)) return 0;

    // Get or set default effect.
    lower_trim_effect = registry[reg_thing_start[THING_LOWER_TRIM]];
    if (!lower_trim_effect) lower_trim_effect = EFFECT_WS_ROTATE;

    // Respond to presses from button 1 (the right one).
    if
    (
        event.type == EVENT_BUTTON &&
        event.data.button.num == 1 &&
        event.data.button.type == BUTTON_PRESSED
    )
    {
        // Rotate through the effects.
        uint8_t next_trim_effect = lower_trim_effect;

        switch (lower_trim_effect)
        {
            case EFFECT_WS_COMET:
                next_trim_effect = EFFECT_WS_ROTATE;
                break;
            case EFFECT_WS_ROTATE:
                next_trim_effect = EFFECT_WS_COMET;
                break;
            default:
                next_trim_effect = EFFECT_WS_ROTATE;
                break;
        }

        registry[reg_thing_start[THING_LOWER_TRIM]] = next_trim_effect;

        Event_t Lower_Trim_Reinit = {
            .type = EVENT_INIT,
            .thing = THING_LOWER_TRIM
        };

        eventPush(Lower_Trim_Reinit);

        return 0;
    }

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
