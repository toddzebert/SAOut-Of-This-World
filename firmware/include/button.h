#ifndef BUTTON_H_
#define BUTTON_H_

// @todo Move to things/button?

#include <stdio.h>
#include <stdbool.h>

#include "global.h"

#define BUTTON_NUM 2

typedef enum {
    BUTTON_SM_WAIT_FOR_START,
    BUTTON_SM_COUNT_LONG_PRESS,
    BUTTON_SM_COUNT_DOUBLE_PRESS,
    BUTTON_SM_WAIT_FOR_STOP
} __attribute__ ((__packed__)) Button_State_Machine_t;

typedef struct 
{
    Button_State_Machine_t machine_state;
    bool debounced_state;
    // These 2 detect our debounced state.
    uint8_t press_count;
    uint8_t release_count;
    // This detects long and double clicks.
    uint16_t hold_count;
} Button_State_t;

// Base is in tocks (1ms).
#define DEBOUNCE_TIMER_BASE 1  // Every 1ms.

// Counts are (required_ms / (BASE * 1ms)).
#define DEBOUNCE_DELAY_COUNT 10  // Stable for 10ms.
#define LONG_PRESS_DELAY_COUNT 1000  // After 500ms.
#define DOUBLE_PRESS_DELAY_COUNT 250  // Within 250ms.

void buttonHandler(Event_t event);

#endif /* BUTTON_H_ */
