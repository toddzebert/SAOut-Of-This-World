#ifndef STARS_H_
#define STARS_H_

#include "../global.h"
#include "../effects.h"
#include <stdio.h>

extern uint16_t thing_tock_timer[THING_COUNT];

extern const uint8_t reg_thing_start[THING_COUNT];

extern uint8_t stars_effect;

int starsHandler(Event_t event);

#endif /* STARS_H_ */
