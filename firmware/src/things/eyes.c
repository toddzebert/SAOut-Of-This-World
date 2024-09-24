#include "things/eyes.h"

uint16_t eyes_timer;
uint8_t eyes_effect;

// @todo this needs to account for differnt effects - locked to blink for now.
int eyesHandler(int flag)
{
    // printf("In eyesHandler, flag is: %d\r\n", flag); // @debug
    eyes_effect = registry[REG_EYES_START] | EFFECT_WS_BLINK;
    // @todo? eyes_timer = 
    switch (eyes_effect)
    {
    case EFFECT_WS_BLINK:
        return effect_ws_blink(THING_EYES, flag);
        break;
    
    default:
        // @todo what to do if given invalid effect?
        break;
    }

    return 0;
}
