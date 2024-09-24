#ifndef EYES_H_
#define EYES_H_

#include "../global.h"
#include "../effects.h"
#include <stdio.h>

// #pragma message ("In eyes.h") // @debug

// #define EYES_TIMER_BASE 5 // @todo needed?

extern uint16_t eyes_timer; // @todo needed?
extern uint8_t eyes_effect; // @todo needed?

int eyesHandler(int flag);

#endif /* EYES_H_ */
