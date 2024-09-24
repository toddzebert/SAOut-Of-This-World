#ifndef EYES_H_
#define EYES_H_

#include "../global.h"
#include "../effects.h"
#include <stdio.h>

// #pragma message ("In eyes.h") // @debug

#define EYES_TIMER_BASE 5

extern uint16_t eyes_timer; // @todo is this needed?
extern uint8_t eyes_state; // @todo is this needed?

void eyesInit(void);

void eyesHandler(void);

#endif /* EYES_H_ */
