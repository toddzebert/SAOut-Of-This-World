#ifndef TWINKLE_H_
#define TWINKLE_H_

#include <stdio.h>

#include "../global.h"

extern uint16_t thing_timer[THING_COUNT];

extern const uint8_t reg_thing_start[THING_COUNT];

extern const uint8_t reg_thing_led_start[THING_COUNT];

int effect_twinkle(Things_t thing, int flag);

#endif /* TWINKLE_H_ */