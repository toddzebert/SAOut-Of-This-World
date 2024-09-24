#include "effects/ws_blink.h"

#include <rand.h>

enum Blink_mode {
    Blink_Random = 1,
};


// Offsets from each "thing" base in the registry.
// @todo these only apply to Eyes!
#define Blink_Effect_offset 0
#define Blink_Timer_offset 1 // 2 bytes
#define Blink_LEDs_offset 3
#define Blink_LEDs_offset_length 6
#define Blink_Left_offset 3
#define Blink_Left_R_offset 3
#define Blink_Left_G_offset 4
#define Blink_Left_B_offset 5
#define Blink_Right_offset 6
#define Blink_Right_R_offset 6
#define Blink_Right_G_offset 7
#define Blink_Right_B_offset 8
#define Blink_Mode_offset 9 // This will be enum so have to cast both ways.
#define Blink_Frequency_offset 10

uint8_t blink_defaults[11] = {
    EFFECT_WS_BLINK,
    0, // Blink_Timer_default_H = 0;
    150, // Blink_Timer_default_L = 150; // Ave eye blink duration.
    0x6a, // Blink_Left_R_default = 0x6A; // #6A5ACD (Slate Blue)
    0x5a, // Blink_Left_G_default = 0x5a;
    0xcd, // Blink_Left_B_default = 0xCD;
    0x7b, // Blink_Right_R_default = 0x7B; // #7B68EE (Medium Slate Blue)
    0x68, // Blink_Right_G_default = 0x68;
    0xee, // Blink_Right_B_default = 0xEE;
    (uint8_t) Blink_Random, // Blink_Mode_default
    15, // Blink_Frequency_default = 15;
};

struct {
    char left_blink : 1;
    char right_blink : 1;
} Blink_state;

uint8_t r; // For random.

/**
 * @brief Blink effect.
 *
 * @param thing
 * @param flag
 * 1: Copy defaults into registry, including effect id.
 * 0: Do the blinking effect.
 */
int effect_ws_blink(Things_t thing, int flag)
{   
    // printf("In effect_ws_blink, thing is: %u, flag is: %d\r\n", thing, flag); // @debug = 1 for Eyes, from Things_t in global.h.
    // printf("thing is: %u\n", thing); // @debug = 1 for Eyes, from Things_t in global.h.

    // @todo take `thing` in account!

    if (flag == 1) { // @todo use an enum for future use?
        Blink_state.left_blink = 0;
        Blink_state.right_blink = 0;
        // Copy defaults to registry.
        // @debug ***** below hard coded to Eyes, but needs to be dynamic. 
        arrayToRegCopy(registry, REG_EYES_START, blink_defaults, 0, sizeof(blink_defaults) * sizeof(uint8_t));
        // Set "raw" reg LED settings from defaults.
        regToRegCopy(registry, REG_EYES_LED_START, registry, REG_EYES_START + Blink_LEDs_offset, Blink_LEDs_offset_length * sizeof(uint8_t));
    }
    else
    {
        // @todo @future check reg mode if more than 1 option.
        
        // Left eye.
        if (Blink_state.left_blink) {
            // printf("Left eye blinking\n"); // @debug
            // Left eye is blinking.
            // Copy left reg to reg raw.
            regToRegCopy(registry, REG_EYES_LEFT_LED_START, registry, REG_EYES_START + Blink_Left_offset, 3 * sizeof(uint8_t));
            // Clear status.
            Blink_state.left_blink = 0;
        }
        else
        {
            // printf("Left eye not blinking\n"); // @debug
            // Maybe left should be blinking?
            r = (uint8_t) rnd_fun(0, 8);

            if (r < registry[REG_EYES_START + Blink_Frequency_offset]) {
                // Set raw reg LED color to black (blink).
                for (int i = 0; i < 3; i++) {
                    registry[REG_EYES_LEFT_LED_START + i] = 0;
                }

                // Set status.
                Blink_state.left_blink = 1;
            }
        }

        // Right eye.
        if (Blink_state.right_blink) {
            // Right eye is blinking.
            // Copy right reg to reg raw.
            regToRegCopy(registry, REG_EYES_RIGHT_LED_START, registry, REG_EYES_START + Blink_Right_offset, 3 * sizeof(uint8_t));
            // Clear status.
            Blink_state.right_blink = 0;
        }
        else
        {
            // Maybe right should be blinking?
            r = (uint8_t) rnd_fun(0, 8);

            if (r < registry[REG_EYES_START + Blink_Frequency_offset]) {
                // Set raw reg LED color to black (blink).
                for (int i = 0; i < 3; i++) {
                    registry[REG_EYES_RIGHT_LED_START + i] = 0;
                }

                // Set status.
                Blink_state.right_blink = 1;
            }
        }
    }

    // Set timer.
    eyes_timer = registry[REG_EYES_START + Blink_Timer_offset] * 256 + registry[REG_EYES_START + Blink_Timer_offset + 1];
    // printf("Eyes timer: %d\n", eyes_timer); // @debug
    // printf("Blink state: "); printBin(blink_state, 1); // @debug
    // printf("Blink state: "); printBinByRef(&Blink_state, 1); // @debug
    // printNon0Reg(registry); // @debug
    return 1; // "Dirty"
}
