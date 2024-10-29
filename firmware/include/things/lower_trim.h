#ifndef LOWER_TRIM_H_
#define LOWER_TRIM_H_

#include "../global.h"
#include "../effects.h"
#include <stdio.h>

extern uint16_t thing_timer[THING_COUNT];

extern const uint8_t reg_thing_start[THING_COUNT];

extern uint8_t lower_trim_effect;

int lowerTrimHandler(Event_t event);

#endif /* LOWER_TRIM_H_ */
