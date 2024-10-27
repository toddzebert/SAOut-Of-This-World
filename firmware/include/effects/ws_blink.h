#ifndef WS_BLINK_H_
#define WS_BLINK_H_

#include <stdio.h>

#include "../global.h"

extern uint16_t thing_timer[THING_COUNT];

extern const uint8_t reg_thing_start[THING_COUNT];

extern const uint8_t thing_led_count[];

int effect_ws_blink(Things_t thing, Event_t event);

#endif /* WS_BLINK_H_ */
