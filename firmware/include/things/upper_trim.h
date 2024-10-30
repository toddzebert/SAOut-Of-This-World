#ifndef UPPER_TRIM_H_
#define UPPER_TRIM_H_

#include "../global.h"
#include "../effects.h"
#include <stdio.h>

extern uint16_t thing_tock_timer[THING_COUNT];

extern const uint8_t reg_thing_start[THING_COUNT];

extern uint8_t upper_trim_effect;

int upperTrimHandler(Event_t event);

#endif /* UPPER_TRIM_H_ */
