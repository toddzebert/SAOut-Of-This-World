#ifndef RAW_H_
#define RAW_H_

#include <stdio.h>

#include "../global.h" // Need for Things_t, etc.

extern uint16_t thing_tock_timer[THING_COUNT];

extern const uint8_t reg_thing_start[THING_COUNT];

int effect_raw(Things_t thing, Event_t event);

#endif /* RAW_H_ */
