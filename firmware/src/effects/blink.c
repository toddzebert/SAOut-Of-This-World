#include "effects/blink.h"
// #include "global.h" // @debug

// #include <stdio.h> // @debug

// #include <ch32v003fun.h>

// int blink_debug = BLINK_DEBUG; // @debug
// int global_debug = GLOBAL_DEBUG; // @debug

enum Blink_mode {
    Blink_Random = 1,
};

/* @debug turns out casting registry arraay to struct is bad coding practice
struct Blink_struct {
    uint16_t timer; // 2 bytes
    uint8_t left_R;
    uint8_t left_G;
    uint8_t left_B;
    uint8_t right_R;
    uint8_t right_G;
    uint8_t right_B;
    enum Blink_mode mode; // this is an int/2 bytes.
    uint8_t frequency;
};
*/

// Offsets from each "thing" base in the registry.
#define Blink_Effect_offset = 0;
#define Blink_Timer_offset = 1; // 2 bytes
#define Blink_Left_R_offset = 3;
#define Blink_Left_G_offset = 4;
#define Blink_Left_B_offset = 5;
#define Blink_Right_R_offset = 6;
#define Blink_Right_G_offset = 7;
#define Blink_Right_B_offset = 8;
#define Blink_Mode_offset = 9; // This will be enum so have to cast both ways.
#define Blink_Frequency_offset = 10;

// @todo this section probably delete?
#define Blink_Timer_default_H = 0;
#define Blink_Timer_default_L = 150; // Ave eye blink duration.
#define Blink_Left_R_default = 0x6A; // #6A5ACD (Slate Blue)
#define Blink_Left_G_default = 0x5a;
#define Blink_Left_B_default = 0xCD;
#define Blink_Right_R_default = 0x7B; // #7B68EE (Medium Slate Blue)
#define Blink_Right_G_default = 0x68;
#define Blink_Right_B_default = 0xEE;
#define Blink_Mode_default = (uint8_t) Blink_Random;
#define Blink_Frequency_default = 10; 

uint8_t blink_defaults[11] = {
    EFFECT_BLINK,
    0, // Blink_Timer_default_H = 0;
    150, // Blink_Timer_default_L = 150; // Ave eye blink duration.
    0x6a, // Blink_Left_R_default = 0x6A; // #6A5ACD (Slate Blue)
    0x5a, // Blink_Left_G_default = 0x5a;
    0xcd, // Blink_Left_B_default = 0xCD;
    0x7b, // Blink_Right_R_default = 0x7B; // #7B68EE (Medium Slate Blue)
    0x68, // Blink_Right_G_default = 0x68;
    0xee, // Blink_Right_B_default = 0xEE;
    (uint8_t) Blink_Random, // Blink_Mode_default
    10, // Blink_Frequency_default = 10; 
};


/**
 * @brief Blink effect.
 *
 * @param thing
 * @param flag
 * 1: Copy defaults into registry, including effect id.
 * 0: Do the blinking effect.
 */
void effect_blink(Things_t thing, int flag)
{   
    printf("In effect_blink\r\n");
    printf("Size of int: %u bytes\n", sizeof(int)); // @debug
    printf("thing is: %u\n", thing); // @debug

    // @todos
    static uint8_t blink_state = 0;

    if (flag == 1) { // @todo use a enum for future use.
        // copy defaults to registry.
        // +1 is to skip Blink_Effect registry entry.
        // Also sets effect id.
        // @todo *******************
        // passing argument 1 of 'memcpy' discards 'volatile' qualifier from pointer target type [-Wdiscarded-qualifiers]
        // memcpy((volatile uint8_t)(registry + REG_EYES_START), blink_defaults, sizeof(blink_defaults) * sizeof(uint8_t));
        // @debug ***** below hard coded to Eyes, but needs to be dynamic. 
        regCopy(registry, REG_EYES_START, blink_defaults, 0, sizeof(blink_defaults) * sizeof(uint8_t));
    }

    // *todo
}
