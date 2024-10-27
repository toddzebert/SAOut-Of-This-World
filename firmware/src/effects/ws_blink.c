#include "effects/ws_blink.h"

#include <rand.h>

enum Blink_mode {
    Blink_Random = 1,
    Blink_Alternate = 2, // Do at timer speed?
    // fade in/out? - we'd have to temp change timer to get any kind of fade.
    //    right now at 150ms, 30fps = 33.333ms, or 15fps 67.777ms. Maybe 40ms, and change overall to 160ms?
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

const uint8_t blink_defaults[11] = {
    EFFECT_WS_BLINK,
    0, // Blink_Timer_default_H = 0;
    150, // Blink_Timer_default_L = 150; // Ave. eye blink duration.
    0x3a, // Blink_Left_R_default = 0x6A; // #6A5ACD (Slate Blue)
    0x8a, // Blink_Left_G_default = 0x5a;
    0xcd, // Blink_Left_B_default = 0xCD;
    0x7b, // Blink_Right_R_default = 0x7B; // #7B68EE (Medium Slate Blue)
    0x58, // Blink_Right_G_default = 0x68;
    0xee, // Blink_Right_B_default = 0xEE;
    (uint8_t) Blink_Alternate, // @debug was Blink_Random, // Blink_Mode_default
    15, // Blink_Frequency_default = 15;
};

struct Blink_State_Default_t {
    char data;
};

struct Blink_State_Random_t {
    char left_blink : 1;
    char right_blink : 1;
};

struct Blink_State_Alternate_s {
    char left_blink : 2; // 0, nothing; 1, blink closing; 2, closed; 3, blink opening  
    char right_blink : 2; // As above.
    char mini_timer: 3; // 0-7.
};

union Blink_State_u {
    struct Blink_State_Default_t def;
    struct Blink_State_Random_t random;
    struct Blink_State_Alternate_s alternate;
};

// @debug old struct Blink_State_t Blink_state[THING_COUNT];

union Blink_State_u Blink_state[THING_COUNT];

State_Action_t state_action[THING_COUNT];

// Functions.
int blinkRandom(Things_t thing, Event_t event);

int blinkAlternate(Things_t thing, Event_t event);

// @debug not needed? int effect_ws_blink_init(Things_t thing, Event_t event);

// @debug not needed? int effect_ws_blink_run(Things_t thing, Event_t event);

// @todo int effect_ws_blink_reg_change(Things_t thing, Event_t event);

// @todo int effect_ws_blink_button(Things_t thing, Event_t event);

/**
 * @brief Blink effect.
 *
 * @param thing
 * @param flag
 * 1: Copy defaults into registry, including effect id.
 * 0: Do the blinking effect.
 */
int effect_ws_blink(Things_t thing, Event_t event)
{   
    printf("In effect_ws_blink, thing: %d, event.type: %d\r\n", thing, event.type); // @debug

    switch (event.type)
    {
    case EVENT_INIT:
        // @note Doesn't need to use event_action.
        // @todo should some of this be in in state_action_init durign first "run"?
        Blink_state[thing].random.left_blink = 0;
        Blink_state[thing].random.right_blink = 0;
        // Copy defaults to registry. 
        constToRegCopy(registry, reg_thing_start[thing], blink_defaults, 0, sizeof(blink_defaults) * sizeof(uint8_t));
        // Set "raw" reg LED settings from defaults. Assumes Things Eyes.
        regToRegCopy(registry, REG_EYES_LED_START, registry, REG_EYES_START + Blink_LEDs_offset, Blink_LEDs_offset_length * sizeof(uint8_t));
        
        state_action[thing] = STATE_ACTION_ENTER; // @todo needed?
        thing_timer[thing] = 10; // Get this moving to Run soon.
        printNon0Reg(registry); // @debug

        return 0;

    case EVENT_RUN:
        printf("In effect_ws_blink_run, thing: %d, event.type: %d\r\n", thing, event.type); // @debug
        // return effect_ws_blink_run(thing, event);
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

    default:
        return 0;
    }
}


int blinkRandom(Things_t thing, Event_t event)
{
    // printf("In blinkRandom, thing: %d, event.type: %d\r\n", thing, event.type); // @debug
    // @todo Doesn't need to use event_action?
    uint8_t r; // For random.

    // Left eye.
    if (Blink_state[thing].random.left_blink) {
        // Left eye is blinking.
        // Copy left reg to reg raw, to unblink. Assumes Things Eyes.
        regToRegCopy(registry, REG_EYES_LEFT_LED_START, registry, REG_EYES_START + Blink_Left_offset, 3 * sizeof(uint8_t));
        // Clear status.
        Blink_state[thing].random.left_blink = 0;
    }
    else
    {
        // Maybe left should be blinking?
        r = (uint8_t) rnd_fun(0, 8);

        if (r < registry[REG_EYES_START + Blink_Frequency_offset]) {
            // Set raw reg LED color to black (blink).
            for (int i = 0; i < 3; i++) {
                registry[REG_EYES_LEFT_LED_START + i] = 0;
            }

            // Set status.
            Blink_state[thing].random.left_blink = 1;
        }
    }

    // Right eye.
    if (Blink_state[thing].random.right_blink) {
        // Right eye is blinking.
        // Copy right reg to reg raw.
        regToRegCopy(registry, REG_EYES_RIGHT_LED_START, registry, REG_EYES_START + Blink_Right_offset, 3 * sizeof(uint8_t));
        // Clear status.
        Blink_state[thing].random.right_blink = 0;
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
            Blink_state[thing].random.right_blink = 1;
        }
    }

    // Set timer.
    thing_timer[thing] = registry[reg_thing_start[thing] + Blink_Timer_offset] * 256 + registry[reg_thing_start[thing] + Blink_Timer_offset + 1];
    
    return 1; // "Dirty".
}

int blinkAlternate(Things_t thing, Event_t event)
{
    printf("In blinkAlternate, thing: %d, event.type: %d\r\n", thing, event.type); // @debug
    // @todo Doesn't need to use event_action?
    // Left eye.
    if (Blink_state[thing].alternate.left_blink) {
        // Left eye is blinking.
        /*
        leave close for x
        then change timer lower (shift >> 2 for 4?)
        change eye color based on eyes default + alpha
        etc....
        */
        
       if (Blink_state[thing].alternate.mini_timer == 0) {
          // @todo 
       }
       else
       {
           // @todo
       }

        // Copy left reg to reg raw, to unblink. Assumes Things Eyes.
        regToRegCopy(registry, REG_EYES_LEFT_LED_START, registry, REG_EYES_START + Blink_Left_offset, 3 * sizeof(uint8_t));
        // Clear status.
        Blink_state[thing].alternate.left_blink = 0;
    }
    // @todo

    return 1; // "Dirty".
}
