#ifndef EYES_H_
#define EYES_H_

#include "../global.h"
#include "../effects.h"
#include <stdio.h>

extern uint16_t thing_tock_timer[THING_COUNT];

extern const uint8_t reg_thing_start[THING_COUNT];

extern uint8_t eyes_effect;

int eyesHandler(Event_t event);

#endif /* EYES_H_ */
