#include "things/eyes.h"

// uint16_t eyes_timer; // @todo be replaced by new array.
uint8_t eyes_effect;

State_Action_t eyes_state_phase;

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
int eyesHandler(Event_t event) // was int flag
{
    // printf("In eyesHandler, event.type: %d\r\n", event.type); // @debug
    // printf("In eyesHandler, registry[REG_EYES_START]: %d\r\n", registry[REG_EYES_START]); // @debug
    int eyes_effect = registry[REG_EYES_START];
    if (eyes_effect == 0) eyes_effect = EFFECT_WS_BLINK;

    // @note In things we can probably ignore a thing state_action.

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
