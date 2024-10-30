#include "effects/ws_rotate.h"

#include <rand.h>
#include <color_utilities.h>

#define WS_Rotate_Timer_offset 1 // 2 bytes
#define WS_Rotate_color_offset 3
#define WS_Rotate_Fixed_Red_offset 3
#define WS_Rotate_Fixed_Green_offset 4
#define WS_Rotate_Fixed_Blue_offset 5
#define WS_Rotate_BG_color_offset 6
#define WS_Rotate_BG_Red_offset 6
#define WS_Rotate_BG_Green_offset 7
#define WS_Rotate_BG_Blue_offset 8
#define WS_Rotate_FG_alpha_offset 9
#define WS_Rotate_Mode_offset 10

const uint8_t ws_rotate_defaults[] = {
    EFFECT_WS_ROTATE,
    0, // Timer_default_H = 0;
    150, // Timer_default_L = TBD;
    0, // Rotate_Red_default = 0;
    0, // Rotate_Green_default = 0;
    0, // Rotate_Blue_default = 0;
    0, // Rotate_BG_Red_default = 0;
    0, // Rotate_BG_Green_default = 0;
    255, // Rotate_BG_Blue_default = 0; // @debug
    255, // Alpha_default = 255;
    0x21, // Mode_default = 0x22; // = Random color + direction.
};


typedef struct 
{
    // Yes, it's reversed.
    uint8_t ln: 4; // dir.
    uint8_t hn: 4; // color.
} WS_Rotate_Mode_Data_t;

typedef union
{
    // This is an ugly way of doing it, but it works.
    uint8_t raw; // to force data in from reg.
    WS_Rotate_Mode_Data_t data;
} WS_Rotate_Mode_t;

// @todo this will have to be fleshed out.
uint8_t WS_Rotate_state[THING_COUNT];

int effect_ws_rotate_run(Things_t thing, Event_t event);

int effect_ws_rotate(Things_t thing, Event_t event)
{
    // printf("In effect_ws_rotate, thing is: %u\r\n", thing); // @debug

    switch (event.type)
    {
        case EVENT_INIT:
            // Copy defaults to registry.
            constToRegCopy(registry, reg_thing_start[thing], ws_rotate_defaults, 0, sizeof(ws_rotate_defaults) * sizeof(uint8_t));

            state_action[thing] = STATE_ACTION_ENTER;
            thing_tock_timer[thing] = 10; // Get this moving to Run soon.

            return 0;

        case EVENT_RUN:
            return effect_ws_rotate_run(thing, event);

        case EVENT_REG_CHANGE:
            // @todo

        default:
            return 0;
    }
}

int effect_ws_rotate_run(Things_t thing, Event_t event)
{
    uint8_t reg_led_temp[27]; // max leds, 9 * 3 (RGB).
    WS_Rotate_Mode_t mode;
    uint8_t r, g, b;

    // printf("In effect_ws_rotate_run, thing is: %u\r\n", thing); // @debug

    if (state_action[thing] == STATE_ACTION_ENTER)
    {
        // @todo set any state here.
        WS_Rotate_state[thing] = mode.data.ln; // @debug for now, just direction.
        if (WS_Rotate_state[thing] == 2)
        {
            // Random dir request.
            WS_Rotate_state[thing] = rnd_fun(0, 1);
        }

        state_action[thing] = STATE_ACTION_GO;
        thing_tock_timer[thing] = 10; // Come back soon.
    }

    // Save the current LED array to the temp array.
    regToArrayCopy(reg_led_temp, 0, registry, reg_thing_led_start[thing], thing_led_count[thing] * 3);

    // Prepare new LED. //
    // Set mode from the registry.
    mode.raw = registry[reg_thing_start[thing] + WS_Rotate_Mode_offset];

    // printf("Mode: 0x%02x\r\n", mode.raw); // @debug
    // printf("Mode.data.hn: 0x%01x\r\n", mode.data.hn); // @debug
    // printf("Mode.data.ln: 0x%01x\r\n", mode.data.ln); // @debug

    // @todo check if Random is anything other than 255, otherwise randomly set to background color.
    // Determine new color's base.
    switch (mode.data.hn)
    {
        case 0: // Fixed color from reg "fixed" color.
            r = registry[reg_thing_start[thing] + WS_Rotate_Fixed_Red_offset];
            g = registry[reg_thing_start[thing] + WS_Rotate_Fixed_Green_offset];
            b = registry[reg_thing_start[thing] + WS_Rotate_Fixed_Blue_offset];
            break;

        case 1: // Complimentary color from reg "fixed" color.
            r = registry[reg_thing_start[thing] + WS_Rotate_Fixed_Red_offset];
            if (rnd_fun(0, 1)) r = 255 - r;
            g = registry[reg_thing_start[thing] + WS_Rotate_Fixed_Green_offset];
            if (rnd_fun(0, 1)) g = 255 - g;
            b = registry[reg_thing_start[thing] + WS_Rotate_Fixed_Blue_offset];
            if (rnd_fun(0, 1)) b = 255 - b;
            break;

        case 2: // Random color.
            r = (uint8_t) rnd_fun(0, 255);
            g = (uint8_t) rnd_fun(0, 255);
            b = (uint8_t) rnd_fun(0, 255);
            break;
    }

    // printf("Color: 0x%02x%02x%02x\r\n", r, g, b); // @debug

    // If alpha, update color.
    if (registry[reg_thing_start[thing] + WS_Rotate_FG_alpha_offset] < 255) {
        // Use BG color from registry.
        uint32_t rgb = blendHexColorsWithAlpha(registry[reg_thing_start[thing] + WS_Rotate_BG_Red_offset], registry[reg_thing_start[thing] + WS_Rotate_BG_Green_offset], registry[reg_thing_start[thing] + WS_Rotate_BG_Blue_offset], r, g, b, registry[reg_thing_start[thing] + WS_Rotate_FG_alpha_offset]);
        r = rgb >> 16;
        g = rgb >> 8;
        b = rgb;
    }

    // Based on direction, copy some parts of temp to LED reg, then populate remaining with new color.
    if (WS_Rotate_state[thing] == 0)
    {
        // Left (lower). So thing's 0th LED gets 2nd of temps.
        arrayToRegCopy(registry, reg_thing_led_start[thing], reg_led_temp, 3, (thing_led_count[thing] - 1) * 3);
        // Populate last LED with new color.
        registry[reg_thing_led_start[thing] + (thing_led_count[thing] - 1) * 3] = r;
        registry[reg_thing_led_start[thing] + (thing_led_count[thing] - 1) * 3 + 1] = g;
        registry[reg_thing_led_start[thing] + (thing_led_count[thing] - 1) * 3 + 2] = b;
        
    }
    else
    {
        // Right (higher). So thing's last LED gets temps 2nd to last.
        arrayToRegCopy(registry, reg_thing_led_start[thing] + 3, reg_led_temp, 0, (thing_led_count[thing] - 1) * 3);
        // Populate first LED with new color.
        registry[reg_thing_led_start[thing]] = r;
        registry[reg_thing_led_start[thing] + 1] = g;
        registry[reg_thing_led_start[thing] + 2] = b;
        
    }

    thing_tock_timer[thing] = registry[reg_thing_start[thing] + WS_Rotate_Timer_offset] * 256 + registry[reg_thing_start[thing] + WS_Rotate_Timer_offset + 1];

    return 1; // @note Routine is always dirty.... for now.
}