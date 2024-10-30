#include "button.h"
#include <stdio.h>

#include <ch32v003fun.h>

uint16_t button1_timer;
uint8_t button1_state;

void button1Init ( void ) {
    // printf("In button1Init\r\n"); // @debug
    
    // @debug old: GPIO_port_enable(GPIO_port_C);
    // @debug no version of this needed with fun*, I think...

    // PC3 for button1.
    // @note changed to pullDown.    
    // @debug old: GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 3), GPIO_pinMode_I_pullDown, GPIO_Speed_In);
    funPinMode( PC3, GPIO_CNF_IN_PUPD );
    funDigitalWrite ( PC3, FUN_LOW ); // @note make it pull down, I think....
    button1_timer = BUTTON_TIMER_BASE;
    button1_state = 0;
}

void button1Handler ( void ) {
    // printf("In button1Handler\r\n"); // @debug
        /*
        int test = GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_C, 3)); // @debug
        if ( test ) {
            printf("button1 !!: %d\r\n", test); // @debug
            //while (1) { } // debug.
        }
        */

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
}
