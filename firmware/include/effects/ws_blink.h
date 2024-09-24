#ifndef WS_BLINK_H_
#define WS_BLINK_H_

#include <stdio.h>

#include "../global.h" // Need for Things_t, etc.

void effect_ws_blink(Things_t thing, int flag);

extern uint16_t eyes_timer;

#endif /* WS_BLINK_H_ */
