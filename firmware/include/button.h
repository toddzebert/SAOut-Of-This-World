#ifndef BUTTON_H_
#define BUTTON_H_

// @todo Move to things/button?

#include <stdio.h>
#include <stdbool.h>

#include "global.h"

#define BUTTON_NUM 2

typedef struct
{
    bool state: 1;
    // bool state_prev: 1;
} Button_State_t;

typedef struct 
{
    uint16_t press_count;
    uint16_t release_count;
    uint16_t debounce_count;
} Button_Debounce_Timers_t;

// Base is in tocks (1ms).
#define DEBOUNCE_TIMER_BASE 5
#define DEBOUNCE_DELAY_COUNT 5
#define LONG_PRESS_DELAY_COUNT 200
#define DOUBLE_PRESS_DELAY_COUNT 40


/* @todo old old for s/o comment.
#define BUTTON_TIMER_BASE 5
// @todo these debounce intervals need some tweaking.
#define BUTTON_ON_ATLEAST 20
#define BUTTON_OFF_ATLEAST 30
*/

/* @todo worked ok
// @see https://stackoverflow.com/q/48434575/25024766
#define BUTTON_CHECK_MSEC 5 // Read hardware every 5 msec
// @note the next two must be divisible by BUTTON_CHECK_MSEC.
#define BUTTON_PRESS_MSEC 25 // Stable time before registering pressed
#define BUTTON_RELEASE_MSEC 150 // @debug was 100 /// Stable time before registering released
*/

/* old old
extern uint16_t button1_timer;
extern uint8_t button1_state; // @todo TBD button function.
*/

Event_t buttonHandler(Event_t event);

#endif /* BUTTON_H_ */
