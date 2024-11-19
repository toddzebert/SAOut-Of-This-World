#include "things/eyes.h"

// uint16_t eyes_timer; // @todo be replaced by new array.
uint8_t eyes_effect;

State_Action_t eyes_state_phase;

struct {
    uint8_t Global_Mode_brightness: 1;
} Eyes_state;

/**
 * @brief Handler for the eyes thing.
 *
 * This function is a handler for the eyes thing. It reads the effect ID from
 * the registry and calls the appropriate effect function, passing the thing ID
 * and the flag as arguments.
 *
 * @param flag The flag to pass to the effect function. The meaning of the flag
 *             depends on the effect function.
 *
 * @return The return value of the effect function. The meaning of the return
 *         value depends on the effect function.
 */
int eyesHandler(Event_t event)
{
    // printf("In eyesHandler - event.type %d, event.thing: %d\r\n", event.type, event.thing);
    // printf("In eyesHandler, event.type: %d\r\n", event.type); // @debug
    // printf("In eyesHandler, registry[REG_EYES_START]: %d\r\n", registry[REG_EYES_START]); // @debug
    if (!(event.thing == THING_EYES || event.thing == THING_ALL)) return 0;
    
    // Get or set default effect.
    int eyes_effect = registry[REG_EYES_START];
    if (eyes_effect == 0) eyes_effect = EFFECT_WS_BLINK;

    // @note In things we can probably ignore a thing state_action.
    // @todo ... OR do I need a case EVENT_INIT?

    if (event.type == EVENT_GLOBAL && event.data.global.mode == GLOBAL_MODE_BRIGHTNESS)
    {
        // printf("In eyesHandler cond, event.type: %d, ...global.mode: %d, ...global.state: %d\r\n", event.type, event.data.global.mode, event.data.global.state); // @debug

        if (event.data.global.state == 1)
        {
            // Set our special Thing state.
            Eyes_state.Global_Mode_brightness = 1;

            // Clear left eye.
            // @todo could also save current eye color to restore later.
            registry[REG_EYES_LEFT_LED_START + RED_OFFSET] = 0;
            registry[REG_EYES_LEFT_LED_START + GREEN_OFFSET] = 0;
            registry[REG_EYES_LEFT_LED_START + BLUE_OFFSET] = 0;
        }
        else if (event.data.global.state == 0)
        {
            // Leave special Thing state.
            Eyes_state.Global_Mode_brightness = 0;

            // @todo emit init for eyes?
        }

        registry[REG_EYES_LEFT_LED_START + RED_OFFSET] = 0;
        thing_tock_timer[THING_EYES] = 10; // Get things back here quick.

        return 0; // Despite clearing eye on mode entry, as it'll be rendered next go-around.
    }

    // Handle global modes.
    if (Eyes_state.Global_Mode_brightness == 1)
    {
        // Flash left eye red.
        if (registry[REG_EYES_LEFT_LED_START + RED_OFFSET])
            registry[REG_EYES_LEFT_LED_START + RED_OFFSET] = 0;
        else
            registry[REG_EYES_LEFT_LED_START + RED_OFFSET] = 255;

        thing_tock_timer[THING_EYES] = 100;

        return 1;
    }

    // Call desired effect.
    switch (eyes_effect)
    {
        case EFFECT_RAW:
            return effect_raw(THING_EYES, event);

        case EFFECT_WS_BLINK:
            return effect_ws_blink(THING_EYES, event);
        
        default:
            // @todo what to do if given invalid effect?
            break;
    }

    return 0;
}
