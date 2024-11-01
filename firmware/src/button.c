#include "button.h"
#include <stdio.h>

#include <ch32v003fun.h>

const uint8_t button_gpio_pins[BUTTON_NUM] = { PC3, PC4 };

/* worked OK
uint16_t button_timers[BUTTON_NUM];
uint8_t button_state[BUTTON_NUM];
*/

Button_State_t button_state[BUTTON_NUM];
Button_Debounce_Timers_t button_timers[BUTTON_NUM];

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

                button_state[i].state = false;
                // button_state[i].state_prev = false;

                button_timers[i].press_count = 0;
                button_timers[i].release_count = 0;
                button_timers[i].debounce_count = 0;
                
                /* works OK....
                button_timers[i] = BUTTON_RELEASE_MSEC / BUTTON_CHECK_MSEC;
                button_state[i] = 0;
                */

                state_action[THING_BUTTONS] = STATE_ACTION_ENTER;
                // worked ok - thing_tock_timer[THING_BUTTONS] = BUTTON_PRESS_MSEC;
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
    // see https://stackoverflow.com/q/48434575/25024766 .
    for (int i = 0; i < BUTTON_NUM; i++)
    {
        bool current_state = funDigitalRead(button_gpio_pins[i]);
        printf("button %d: %d\r\n", i, current_state); // @debug
        // Debounce the button state
        if (current_state != button_state[i].state)
        {
            if (button_timers[i].debounce_count == 0)
            {
                button_state[i].state = current_state;
                button_timers[i].debounce_count = DEBOUNCE_DELAY_COUNT;
            }
            else button_timers[i].debounce_count--;
        }
        else button_timers[i].debounce_count = 0;

        if (button_state[i].state)
        {
            if (button_timers[i].press_count == 0) button_timers[i].press_count = 1;
            else button_timers[i].press_count++;
        }
        else
        {
            if (button_timers[i].release_count == 0) button_timers[i].release_count = 1;
            else button_timers[i].release_count++;
        }

        if (button_state[i].state && button_timers[i].press_count >= LONG_PRESS_DELAY_COUNT)
        {
            button_timers[i].press_count = 0;
            button_timers[i].release_count = 0;
            printf("button LONG pressed\r\n"); // @debug
            // @todo event...
        }
        else if (!button_state[i].state && button_timers[i].release_count - button_timers[i].press_count <= DOUBLE_PRESS_DELAY_COUNT)
        {
            button_timers[i].press_count = 0;
            button_timers[i].release_count = 0;
            printf("button DOUBLE pressed\r\n"); // @debug
            // @todo event...
        }
        else if (button_state[i].state)
        {
            printf("button pressed\r\n"); // @debug
            // @todo event...
        }
        else if (!button_state[i].state && button_timers[i].release_count > 0)
        {
            printf("button released\r\n"); // @debug
            // @todo event...
        }

    /* works OK....
        uint8_t raw_state = false;
        uint8_t button_changed = false;
        uint8_t button_pressed = button_state[i];

        raw_state = funDigitalRead(button_gpio_pins[i]);

        // @todo? if (state_action[thing] == STATE_ACTION_ENTER)

        if (raw_state == button_state[i])
        {
            // Set the timer which will allow a change from the current state.
            if (button_state[i]) button_timers[i] = BUTTON_RELEASE_MSEC / BUTTON_CHECK_MSEC;
            else button_timers[i] = BUTTON_PRESS_MSEC / BUTTON_CHECK_MSEC;
        }
        else
        {
            // Key has changed - wait for new state to become stable.
            if (--button_timers[i] == 0)
            {
                // Timer expired - accept the change.
                button_state[i] = raw_state;
                button_changed = true;
                printf("button_changed : %d\r\n", button_changed); // @debug
                // @todo *Key_changed=true; // this we can ignore, because sending event
                // *Key_pressed = DebouncedKeyPress;
                // And reset the timer.
                if (button_state[i]) button_timers[i] = BUTTON_RELEASE_MSEC / BUTTON_CHECK_MSEC;
                else button_timers[i] = BUTTON_PRESS_MSEC / BUTTON_CHECK_MSEC;

                printf("button state : %d\r\n", button_state[i]); // @debug
                // @todo return event! and reset the timer first!
            }
        }
    */
    }

    // worked ok - thing_tock_timer[THING_BUTTONS] = BUTTON_PRESS_MSEC;
    thing_tock_timer[THING_BUTTONS] = DEBOUNCE_TIMER_BASE;

    return Event_None;

/*
        // Debounce button.
        // Adapted from https://stackoverflow.com/a/48435065 .
        if (button1_state > 1)
        {
            button1_state -= 2;
            // printf("button1_state >1 : %d\r\n", button1_state); // @debug
        }
        else
        // @note with pulldown, there's a short periodvat startup when button is high!
        // @todo switch to `fun built-in GPIO.
        // uint8_t button_is_pressed = !GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_D, 3));
        // @debug old: if ( (!GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_C, 3))) != (!button1_state) ) {
        if ( (!funDigitalRead( PC3 )) != (!button1_state) ) {
            // printf("if != TRUE \r\n"); // @debug
            if (button1_state)
                button1_state = BUTTON_OFF_ATLEAST * 2 + 0;
            else
                button1_state = BUTTON_ON_ATLEAST * 2 + 1;
        }
        if ( button1_state & 1 ) {
            // printf("button1_state &1 TRUE : %d\r\n", button1_state); // @debug
            button1_state = 0;
        }
        // printf("button1_timer reset to BASE\r\n"); // @debug
        button1_timer = BUTTON_TIMER_BASE;
    */
}
