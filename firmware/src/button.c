#include "button.h"
#include <stdio.h>

#include <ch32v003fun.h>

uint16_t button1_timer;
uint8_t button1_state;

void button1Init ( void ) {
    printf("In button1Init\r\n"); // @debug
    
    GPIO_port_enable(GPIO_port_C);

    // PC3 for button1.
    // @note changed to pullDown.    
    GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 3), GPIO_pinMode_I_pullDown, GPIO_Speed_In);

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
        if ( (!GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_C, 3))) != (!button1_state) ) {
            // printf("if != TRUE \r\n"); // @debug
            if (button1_state)
                button1_state = BUTTON_OFF_ATLEAST * 2 + 0;
            else
                button1_state = BUTTON_ON_ATLEAST * 2 + 1;
        }
        if ( button1_state & 1 ) {
            printf("button1_state &1 TRUE : %d\r\n", button1_state); // @debug
            button1_state = 0;

            /* @todo
            switch (comet_selection)    {
                case 0: {
                    comet_colors_current = &comet_colors_1;
                    comet_selection = 1;
                    break;
                    }
                case 1: {
                    comet_colors_current = &comet_colors_0;
                    comet_selection = 0;
                    break;
                    }
            }
            printf("comet_selection: %d\r\n\r\n", comet_selection); // @debug
            */
        }
        // printf("button1_timer reset to BASE\r\n"); // @debug
        button1_timer = BUTTON_TIMER_BASE;
}
