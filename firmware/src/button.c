#include "button.h"
#include <stdio.h>

#include <ch32v003fun.h>

const uint8_t button_gpio_pins[BUTTON_NUM] = { PC3, PC4 };

// Timers and states for each of the buttons.
Button_State_t button_state[BUTTON_NUM];

Event_t buttonHandler_run(Event_t event);

Event_t buttonHandler(Event_t event)
{
    // printf( "In buttonHandler, event.type: %d\r\n", event.type ); // @debug
    if (!(event.thing == THING_BUTTONS || event.thing == THING_ALL)) return Event_None;
    
    switch (event.type)
    {
        case EVENT_INIT:
            // printf( "In buttonHandler EVENT_INIT, event.type: %d\r\n", event.type ); // @debug
            for (int i = 0; i < BUTTON_NUM; i++)
            {
                funPinMode(button_gpio_pins[i], GPIO_CNF_IN_PUPD);
                funDigitalWrite(button_gpio_pins[i], FUN_HIGH);

                // Default vals.
                button_state[i].machine_state = BUTTON_SM_WAIT_FOR_START;
                button_state[i].debounced_state = false;
                button_state[i].press_count = 0;
                button_state[i].release_count = 0;
                button_state[i].hold_count = 0;

                state_action[THING_BUTTONS] = STATE_ACTION_ENTER;
                thing_tock_timer[THING_BUTTONS] = DEBOUNCE_TIMER_BASE;
            }

            return Event_None;

        case EVENT_RUN:
            return buttonHandler_run(event);

        default:
            return Event_None;
    }

    return Event_None;
}

Event_t buttonHandler_run(Event_t event)
{
    for (int i = 0; i < BUTTON_NUM; i++)
    {
        // Since we're using pull-ups, the state will be inverted.
        bool raw_state = !funDigitalRead(button_gpio_pins[i]);
        //printf("button %d: %d\r\n", i, raw_state); // @debug

        // Debounce the button.
        if (raw_state)
        {
            // We see the button is pressed, stop any released counting.
            button_state[i].release_count = 0;
            if (button_state[i].press_count < 255)
                button_state[i].press_count++;
            // If we have been pressed for N consecutive cycles and the
            // debounced state isn't true, then we flip the state.
            if (button_state[i].press_count >= DEBOUNCE_DELAY_COUNT &&
                !button_state[i].debounced_state)
            {
                button_state[i].debounced_state = true;
            }
        }
        else
        {
            // We see the button is not pressed, stop any pressed counting.
            button_state[i].press_count = 0;
            if (button_state[i].release_count < 255)
                button_state[i].release_count++;
            // If we have been released for N consecutive cycles and the
            // debounced state is true, then we flip the state.
            if (button_state[i].release_count >= DEBOUNCE_DELAY_COUNT &&
                    button_state[i].debounced_state)
            {
                button_state[i].debounced_state = false;
            }
        }

        // External event that tells us what the button is doing.
        Button_Event_Type_t press_event = BUTTON_NONE;

        // Detect events for click/long/double.
        bool debounced_state = button_state[i].debounced_state;
        if (button_state[i].machine_state == BUTTON_SM_WAIT_FOR_START)
        {
            if (debounced_state)
            {
                button_state[i].hold_count = 0;
                //printf("BUTTON_SM_WAIT_FOR_START -> BUTTON_SM_COUNT_LONG_PRESS\n");
                button_state[i].machine_state = BUTTON_SM_COUNT_LONG_PRESS;
            }
        }
        if (button_state[i].machine_state == BUTTON_SM_COUNT_LONG_PRESS)
        {
            if (debounced_state)
            {
                button_state[i].hold_count++;
                if (button_state[i].hold_count == LONG_PRESS_DELAY_COUNT)
                {
                    press_event = BUTTON_LONG_PRESSED;
                    //printf("BUTTON_SM_COUNT_LONG_PRESS -> BUTTON_SM_WAIT_FOR_STOP\n");
                    button_state[i].machine_state = BUTTON_SM_WAIT_FOR_STOP;
                }
            }
            else
            {
                button_state[i].hold_count = 0;
                //printf("BUTTON_SM_COUNT_LONG_PRESS -> BUTTON_SM_COUNT_DOUBLE_PRESS\n");
                button_state[i].machine_state = BUTTON_SM_COUNT_DOUBLE_PRESS;
            }
        }
        if (button_state[i].machine_state == BUTTON_SM_COUNT_DOUBLE_PRESS)
        {
            if (!debounced_state)
            {
                button_state[i].hold_count++;
                if (button_state[i].hold_count == DOUBLE_PRESS_DELAY_COUNT)
                {
                    press_event = BUTTON_PRESSED;
                    //printf("BUTTON_SM_COUNT_DOUBLE_PRESS -> BUTTON_SM_WAIT_FOR_START\n");
                    button_state[i].machine_state = BUTTON_SM_WAIT_FOR_START;
                }
            }
            else
            {
                press_event = BUTTON_DOUBLE_PRESSED;
                //printf("BUTTON_SM_COUNT_DOUBLE_PRESS -> BUTTON_SM_WAIT_FOR_STOP\n");
                button_state[i].machine_state = BUTTON_SM_WAIT_FOR_STOP;
            }
        }
        if (button_state[i].machine_state == BUTTON_SM_WAIT_FOR_STOP)
        {
            if (!debounced_state)
            {
                //printf("BUTTON_SM_WAIT_FOR_STOP -> BUTTON_SM_WAIT_FOR_START\n");
                button_state[i].machine_state = BUTTON_SM_WAIT_FOR_START;
            }
        }

        // Print it out just for debug.
        {
            if (press_event == BUTTON_NONE)
            {
            }
            else if (press_event == BUTTON_PRESSED)
            {
                printf("buttonHandler_run: Press event!\n");
            }
            else if (press_event == BUTTON_LONG_PRESSED)
            {
                printf("buttonHandler_run: Long press event!\n");
            }
            else if (press_event == BUTTON_DOUBLE_PRESSED)
            {
                printf("buttonHandler_run: Double press event!\n");
            }
            else
            {
                printf("buttonHandler_run: ERROR! Unhandled external event: %d\n", press_event);
            }
        }
    }

    thing_tock_timer[THING_BUTTONS] = DEBOUNCE_TIMER_BASE;

    return Event_None;
}
