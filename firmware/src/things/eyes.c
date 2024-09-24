#include "things/eyes.h"

uint16_t eyes_timer;
uint8_t eyes_state;
uint8_t eyes_effect;

// @todo this needs to account for differnt effects - locked to blink for now.
void eyesInit(void)
{
    // printf("In eyesInit\r\n");
    eyes_effect = EFFECT_BLINK;
    // eyes_timer = 
    effect_ws_blink(THING_EYES, 1); // To have blink reset.
}

void eyesHandler(void)
{
    // printf("In eyesHandler\r\n"); // @debug
    // @todo
    // set eyes_timer to value from registry...
    // @todo ...?
    effect_ws_blink(THING_EYES, 0); 
}
