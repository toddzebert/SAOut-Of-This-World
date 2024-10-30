#ifndef WS_COMET_H_
#define WS_COMET_H_

#include <stdio.h>

#include "../global.h" // Need for Things_t, etc.

extern uint16_t thing_tock_timer[THING_COUNT];

extern const uint8_t reg_thing_start[THING_COUNT];

extern const uint8_t thing_led_count[];

extern State_Action_t state_action[THING_COUNT];

int effect_ws_comet(Things_t thing, Event_t event);

#endif /* WS_COMET_H_ */
