#include "things/eyes.h"

uint16_t eyes_timer; // @todo be replaced by new array.
uint8_t eyes_effect;

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
int eyesHandler(int flag)
{
    // printf("In eyesHandler, flag is: %d\r\n", flag); // @debug
    eyes_effect = registry[REG_EYES_START] | EFFECT_WS_BLINK;
    // @todo? eyes_timer = 
    switch (eyes_effect)
    {
    case EFFECT_RAW:
        return effect_raw(THING_EYES, flag);
        break;

    case EFFECT_WS_BLINK:
        return effect_ws_blink(THING_EYES, flag);
        break;
    
    default:
        // @todo what to do if given invalid effect?
        break;
    }

    return 0;
}
