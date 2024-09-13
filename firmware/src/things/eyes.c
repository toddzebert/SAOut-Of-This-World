#include "things/eyes.h"

uint16_t eyes_timer;
uint8_t eyes_state;
uint8_t eyes_effect;

// int blink_debug = BLINK_DEBUG; // @debug
// int global_debug = GLOBAL_DEBUG; // @debug


void eyesInit(void)
{
    printf("In eyesInit\r\n");
    eyes_effect = EFFECT_BLINK;
    // @todos
    // init registry from defaults
    effect_blink(THING_EYES, 1); // To have blink reset.
}

void eyesHandler(void)
{
    printf("In eyesHandler\r\n");
    // @todo
    // set eyes_timer to value from registry...
    // *todo

}
