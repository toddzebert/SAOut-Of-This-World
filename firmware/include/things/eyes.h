#ifndef EYES_H_
#define EYES_H_

#include "../global.h"
#include "../effects.h"
#include <stdio.h>

// #pragma message ("In eyes.h") // @debug

extern uint16_t eyes_timer; // @todo to be replaced by new array.
extern uint8_t eyes_effect;

int eyesHandler(int flag);

#endif /* EYES_H_ */
