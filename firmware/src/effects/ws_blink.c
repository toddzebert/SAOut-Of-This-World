#include "effects/ws_blink.h"

#include <rand.h>
#include <color_utilities.h>

enum Blink_mode {
    // @todo default = 0?
    Blink_Random = 1,
    Blink_Alternate = 2,
};

// Offsets from each "thing" base in the registry.
// @todo these only apply to Eyes!
#define Blink_Effect_offset 0
#define Blink_Timer_offset 1 // 2 bytes
#define Blink_LEDs_offset 3
#define Blink_LEDs_offset_length 6
// @todo fix naming - add LED(S).
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

const uint8_t blink_defaults[11] = {
    EFFECT_WS_BLINK,
    // 150ms is Ave. eye blink duration, while 75ms works better for blink_alternate.
    0, // Blink_Timer_default_H = 0;
    75, // Blink_Timer_default_L = 150;
    0x3a, // Blink_Left_R_default = 0x6A; // #6A5ACD (Slate Blue)
    0x8a, // Blink_Left_G_default = 0x5a;
    0xcd, // Blink_Left_B_default = 0xCD;
    0x7b, // Blink_Right_R_default = 0x7B; // #7B68EE (Medium Slate Blue)
    0x58, // Blink_Right_G_default = 0x68;
    0xee, // Blink_Right_B_default = 0xEE;
    (uint8_t) Blink_Alternate, // @todo was Blink_Random, // Blink_Mode_default
    15, // Blink_Frequency_default = 15;
};

// Alternate alpha lookup table.
// @todo this is not actually correct, as to go from 0-255 in 8 steps, inclusive, the increment is close to 37.
const uint8_t blink_alternate_alpha[8] = {0, 32, 64, 96, 128, 160, 192, 255};

struct Blink_State_Default_t {
    char data; // TBD...
};

struct Blink_State_Random_t {
    char left_blink : 1;
    char right_blink : 1;
};

struct Blink_State_Alternate_s {
    // *_blink's: b00, blink open; b01, blink opening; b10, closed; b11, blink closing; and then loop (decrement).
    char left_blink : 2;
    char right_blink : 2;
    char mini_timer: 3; // 0-7.
};

union Blink_State_u {
    struct Blink_State_Default_t def; // @todo used how?
    struct Blink_State_Random_t random;
    struct Blink_State_Alternate_s alternate;
};

union Blink_State_u Blink_state[THING_COUNT];

State_Action_t state_action[THING_COUNT];

// Functions.
int blinkRandom(Things_t thing, Event_t event);

int blinkAlternate(Things_t thing, Event_t event);

// @todo int effect_ws_blink_reg_change(Things_t thing, Event_t event);

// @todo int effect_ws_blink_button(Things_t thing, Event_t event);


/**
 * @brief The main function for the WS2812 blink effect.
 *
 * This function is a handler for the WS2812 blink effect. It reads the effect ID from
 * the registry and calls the appropriate effect function, passing the thing ID and the
 * event as arguments.
 *
 * @param thing The ID of the thing to run the effect on.
 * @param event The event to run the effect with.
 *
 * @return The return value of the effect function. The meaning of the return value
 *         depends on the effect function.
 */
int effect_ws_blink(Things_t thing, Event_t event)
{   
    // printf("In effect_ws_blink, thing: %d, event.type: %d\r\n", thing, event.type); // @debug

    switch (event.type)
    {
    case EVENT_INIT:
        // STATE_ACTION_INIT is handled here, and _ENTER is handled in the subroutines.
        // Copy defaults to registry. 
        constToRegCopy(registry, reg_thing_start[thing], blink_defaults, 0, sizeof(blink_defaults) * sizeof(uint8_t));

        state_action[thing] = STATE_ACTION_ENTER;
        thing_timer[thing] = 10; // Get this moving to Run soon.
        printNon0Reg(registry); // @debug

        return 0;

    case EVENT_RUN:
        // printf("In effect_ws_blink_run, thing: %d, event.type: %d\r\n", thing, event.type); // @debug

        switch (registry[reg_thing_start[thing] + Blink_Mode_offset])
        {
        case Blink_Random:
            return blinkRandom(thing, event);

        case Blink_Alternate:
            return blinkAlternate(thing, event);         
        }

        return 0;

    case EVENT_REG_CHANGE:
        // @note Doesn't need to use event_action.
        // @todo might need to change state back to enter.    
        // @todo effect_ws_blink_reg_change(thing, event);

        return 0;

    // @todo case EVENT_BUTTON: ???

    default:
        return 0;
    }
}


/**
 * @brief Handler for the random blink effect.
 *
 * This function is a handler for the random blink effect. It blinks the LEDs at
 * a random interval, and sets the LED color to black while blinking.
 *
 * @param thing The thing ID.
 * @param event The event that triggered the effect.
 *
 * @return 1 if the LEDs were modified, 0 otherwise.
 */
int blinkRandom(Things_t thing, Event_t event)
{
    // printf("In blinkRandom, thing: %d, event.type: %d\r\n", thing, event.type); // @debug

    uint8_t r; // For random.
    int dirty = 0;

    if (state_action[thing] == STATE_ACTION_ENTER)
    {
        // Set "raw" reg LED settings from defaults. Assumes Things Eyes.
        regToRegCopy(registry, REG_EYES_LED_START, registry, REG_EYES_START + Blink_LEDs_offset, Blink_LEDs_offset_length * sizeof(uint8_t));

        Blink_state[thing].random.left_blink = 0;
        Blink_state[thing].random.right_blink = 0;

        state_action[thing] = STATE_ACTION_GO;
        thing_timer[thing] = 10; // Come back soon.

        return 1; // Dirty.
    }

    // Left eye.
    if (Blink_state[thing].random.left_blink) {
        // Left eye is blinking.
        // Copy left reg to reg raw, to unblink. Assumes Things Eyes.
        regToRegCopy(registry, REG_EYES_LEFT_LED_START, registry, REG_EYES_START + Blink_Left_offset, 3 * sizeof(uint8_t));
        // Clear status.
        Blink_state[thing].random.left_blink = 0;

        dirty = 1;
    }
    else
    {
        // Maybe left should be blinking?
        r = (uint8_t) rnd_fun(0, 8);

        if (r <= registry[REG_EYES_START + Blink_Frequency_offset]) {
            // Set raw reg LED color to black (blink).
            for (int i = 0; i < 3; i++) {
                registry[REG_EYES_LEFT_LED_START + i] = 0;
            }

            dirty = 1;

            // Set status.
            Blink_state[thing].random.left_blink = 1;
        }
    }

    // Right eye.
    if (Blink_state[thing].random.right_blink) {
        // Right eye is blinking.
        // Copy right reg to reg raw.
        regToRegCopy(registry, REG_EYES_RIGHT_LED_START, registry, REG_EYES_START + Blink_Right_offset, 3 * sizeof(uint8_t));
        dirty = 1;
        
        // Clear status.
        Blink_state[thing].random.right_blink = 0;
    }
    else
    {
        // Maybe right should be blinking?
        r = (uint8_t) rnd_fun(0, 8);

        if (r <= registry[REG_EYES_START + Blink_Frequency_offset]) {
            // Set raw reg LED color to black (blink).
            for (int i = 0; i < 3; i++) {
                registry[REG_EYES_RIGHT_LED_START + i] = 0;
            }

            dirty = 1;

            // Set status.
            Blink_state[thing].random.right_blink = 1;
        }
    }

    // Set timer.
    thing_timer[thing] = registry[reg_thing_start[thing] + Blink_Timer_offset] * 256 + registry[reg_thing_start[thing] + Blink_Timer_offset + 1];
    
    return dirty;
}

/**
 * @brief Alternate blink effect for WS2812 eyes.
 *
 * This function is called by effect_ws_blink and handles the alternate blink effect.
 *
 * @param thing The ID of the thing to run the effect on.
 * @param event The event to run the effect with.
 *
 * @return  1 if the LEDs were modified, 0 otherwise.
 * 
 * @note the fade effect is choppy and needs to be reviewed and fixed.
 */
int blinkAlternate(Things_t thing, Event_t event)
{
    int dirty = 0;

    // printf("In blinkAlternate, thing: %d, event.type: %d, state_action: %d\r\n", thing, event.type, state_action[thing]); // @debug

    if (state_action[thing] == STATE_ACTION_ENTER)
    {
        // printf("In blinkAlternate, STATE_ACTION_ENTER.\r\n"); // @debug
        // Set "raw" reg LED (Left only, see state below) settings from defaults. Assumes Things Eyes.
        regToRegCopy(registry, REG_EYES_LED_START, registry, REG_EYES_START + Blink_LEDs_offset, (Blink_LEDs_offset_length / 2) * sizeof(uint8_t));
        // @todo should I set right eye to black?

        // Set state.
        Blink_state[thing].alternate.left_blink = 0; // Open.
        Blink_state[thing].alternate.right_blink = 2; // Closed.
        Blink_state[thing].alternate.mini_timer = 7; // For count down.

        state_action[thing] = STATE_ACTION_GO;
        thing_timer[thing] = 10; // Come back soon.

        return 1; // Dirty.
    }

    // printf("In blinkAlternate, STATE_ACTION_GO+.\r\n"); // @debug
    // Assume STATE_ACTION_GO...
    if (Blink_state[thing].alternate.mini_timer == 0) {
        // printf("Timer done.\r\n"); // @debug
        // Decrement eye blink status, and wrap around if necessary.
        Blink_state[thing].alternate.left_blink = (Blink_state[thing].alternate.left_blink == 0) ? 3 : Blink_state[thing].alternate.left_blink - 1;

        Blink_state[thing].alternate.right_blink = (Blink_state[thing].alternate.right_blink == 0) ? 3 : Blink_state[thing].alternate.right_blink - 1;

        // Reset mini_timer.
        Blink_state[thing].alternate.mini_timer = 7;
    }
    else
    {
        Blink_state[thing].alternate.mini_timer -= 1;
    }

    uint32_t hex_color, hexa;
    uint8_t alpha, r, g, b;

    // Left eye. If it's opening or closing...
    if (Blink_state[thing].alternate.left_blink & 1) {
        // printf("In blinkAlternate, left_blink & 1.\r\n"); // @debug
        hexa = registry[REG_EYES_START + Blink_Left_offset] << 16 | registry[REG_EYES_START + Blink_Left_offset + 1] << 8 | registry[REG_EYES_START + Blink_Left_offset + 2];
        // printf("In blinkAlternate, left_blink, hexa: %ld.\r\n", hexa); // @debug
        // Use 2nd lsb to determine direction of fade for alpha.
        if ((Blink_state[thing].alternate.left_blink >> 1) & 1)
        {
            // b11, blink closing.
            alpha = 255 - blink_alternate_alpha[Blink_state[thing].alternate.mini_timer];
        }
        else
        {
            // b01, blink opening.
            alpha = blink_alternate_alpha[Blink_state[thing].alternate.mini_timer];
        }
        // printf("In blinkAlternate, left_blink, alpha: %d.\r\n", alpha); // @debug
        
        // Alpha applies to hexa.
        hex_color = TweenHexColors(0, hexa, alpha);
        // printf("In blinkAlternate, left_blink, hex_color: %ld.\r\n", hex_color); // @debug

        r = (hex_color >> 16) & 0xFF;
        g = (hex_color >> 8) & 0xFF;
        b = hex_color & 0xFF;

        registry[reg_thing_led_start[thing] + 0] = r;
        registry[reg_thing_led_start[thing] + 1] = g;
        registry[reg_thing_led_start[thing] + 2] = b;

        dirty = 1;
    }

    // Right eye.
    if (Blink_state[thing].alternate.right_blink & 1) {
        // printf("In blinkAlternate, right_blink & 1.\r\n"); // @debug
        hexa = registry[REG_EYES_START + Blink_Right_offset] << 16 | registry[REG_EYES_START + Blink_Right_offset + 1] << 8 | registry[REG_EYES_START + Blink_Right_offset + 2];
        // printf("In blinkAlternate, right_blink, hexa: %ld.\r\n", hexa); // @debug
        // Use 2nd lsb to determine direction of fade for alpha.
        if ((Blink_state[thing].alternate.right_blink >> 1) & 1)
        {
            // b11, blink closing. So at t=0, the lookup alpha is 0, resulting in alpha 255 - 0 = 255.
            alpha = 255 - blink_alternate_alpha[Blink_state[thing].alternate.mini_timer];
        }
        else
        {
            // b01, blink opening.
            alpha = blink_alternate_alpha[Blink_state[thing].alternate.mini_timer];
        }

        // Alpha applies to hexa.
        hex_color = TweenHexColors(0, hexa, alpha);

        r = (hex_color >> 16) & 0xFF;
        g = (hex_color >> 8) & 0xFF;
        b = hex_color & 0xFF;

        // "3" is Right eye offset.
        registry[reg_thing_led_start[thing] + 3 + 0] = r;
        registry[reg_thing_led_start[thing] + 3 + 1] = g;
        registry[reg_thing_led_start[thing] + 3 + 2] = b;

        dirty = 1;
    }
    
    // Set timer.
    thing_timer[thing] = registry[reg_thing_start[thing] + Blink_Timer_offset] * 256 + registry[reg_thing_start[thing] + Blink_Timer_offset + 1];

    return dirty;
}
