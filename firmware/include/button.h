#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdio.h>
#include <ch32v003_GPIO_branchless.h>

#define BUTTON_TIMER_BASE 5
// @todo these debounce intervals need some tweaking.
#define BUTTON_ON_ATLEAST 20
#define BUTTON_OFF_ATLEAST 30

extern uint16_t button1_timer;
extern uint8_t button1_state; // @todo TBD button function.

void button1Init(void);

void button1Handler(void);

#endif /* BUTTON_H_ */
