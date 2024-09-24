#ifndef EYES_H_
#define EYES_H_

#include "../global.h"
#include "../effects.h"
#include <stdio.h>

// #pragma message ("In eyes.h") // @debug

#define EYES_TIMER_BASE 5

extern uint16_t eyes_timer;
extern uint8_t eyes_effect;

int eyesHandler(int flag);

#endif /* EYES_H_ */
