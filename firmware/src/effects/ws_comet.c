#include "effects/ws_comet.h"

#include <rand.h>
#include <color_utilities.h>

#define WS_Comet_Timer_offset 1 // 2 bytes
#define WS_Comet_color_offset 3
#define WS_Comet_Red_offset 3
#define WS_Comet_Green_offset 4
#define WS_Comet_Blue_offset 5
#define WS_Comet_BG_color_offset 6
#define WS_Comet_BG_Red_offset 6
#define WS_Comet_BG_Green_offset 7
#define WS_Comet_BG_Blue_offset 8
#define WS_Comet_Tail_offset 9
#define WS_Comet_Mode_offset 10
#define WS_Comet_Repeats_offset 11
#define WS_Comet_Special_offset 12

const uint8_t ws_comet_defaults[] = {
    EFFECT_WS_COMET,
    1, // Timer_default_H = 0;
    100, // Timer_default_L = TBD;
    255, // Comet_Red_default = 255;
    0, // Comet_Green_default = 0;
    0, // Comet_Blue_default = 0;
    0, // Comet_BG_Red_default = 0;
    0, // Comet_BG_Green_default = 0;
    0, // Comet_BG_Blue_default = 0;
    4, // Tail length default = 4 *** factors of 2 strongly recommended to avoid division. @todo does >8 make any sense?
    0, // Mode_default - 0 = fixed color, 1+ TBD.
    0, // Repeats, default 0
    0,// special?
};

struct WS_Comet_state_t {
    uint8_t position: 4; // 0-15 but we only need max 9.
    uint8_t direction: 1; // 0 = left, 1 = right.
    uint8_t status: 3; // 0-7 - use will depend on the mode. TBD.
};

// @todo make all other WS events store state in thing array.
struct WS_Comet_state_t WS_Comet_state[THING_COUNT];

struct Alpha_Layer_LED_t {
    uint8_t color[3];
    uint8_t alpha; // W/o FP this is a 0-7, for shifting.
    uint8_t mask;
};

int effect_ws_comet_run(Things_t thing, Event_t event);

int effect_ws_comet(Things_t thing, Event_t event)
{
    switch (event.type)
    {
        case EVENT_INIT:
            // Copy defaults to registry.
            constToRegCopy(registry, reg_thing_start[thing], ws_comet_defaults, 0, sizeof(ws_comet_defaults) * sizeof(uint8_t));

            state_action[thing] = STATE_ACTION_ENTER;
            thing_tock_timer[thing] = 10; // Get this moving to Run soon.
            // printNon0Reg(registry); // @debug

            return 0;

        case EVENT_RUN:
            switch (registry[reg_thing_start[thing] + WS_Comet_Mode_offset])
            {
                // @todo
            }
            return effect_ws_comet_run(thing, event);

        case EVENT_REG_CHANGE:
            // @todo
        default:
            return 0;
    }
}


int effect_ws_comet_run(Things_t thing, Event_t event)
{
    // This stays inside so it's not static!
    // @note Set to lower trim as it has the most and excess won't matter.
    struct Alpha_Layer_LED_t alpha_layer_leds[LOWER_TRIM_COUNT] = {}; // Largest Thing size.

    if (state_action[thing] == STATE_ACTION_ENTER)
    {
        // Set initial state.
        WS_Comet_state[thing].position = thing_led_count[thing] >> 2; // Start at middle.
        WS_Comet_state[thing].direction = rnd_fun(0, 1);
        WS_Comet_state[thing].status = 0;
        // printf("WS *init* comet state: thing %d, pos %d, dir %d, status %d\n", thing, WS_Comet_state[thing].position, WS_Comet_state[thing].direction, WS_Comet_state[thing].status); // @debug

        // Apply background color.
        for (int i = 0; i < thing_led_count[thing]; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                registry[reg_thing_led_start[thing] + (i * 3) + j] = registry[reg_thing_start[thing] + WS_Comet_BG_color_offset + j];
            }
        }

        state_action[thing] = STATE_ACTION_GO;
        thing_tock_timer[thing] = 10; // Come back soon.

        return 1; // Dirty.
    }
    
    // Assuming STATE_ACTION_GO.
    // printf("WS comet state: thing %d, pos %d, dir %d, status %d\n", thing, WS_Comet_state[thing].position, WS_Comet_state[thing].direction, WS_Comet_state[thing].status); // @debug

    // Get comet colors handy.
    // @note "curr" is leftover from earlier code, but may mean something with "mode".
    uint8_t curr_led_r = registry[reg_thing_start[thing] + WS_Comet_Red_offset];
    uint8_t curr_led_g = registry[reg_thing_start[thing] + WS_Comet_Green_offset];
    uint8_t curr_led_b = registry[reg_thing_start[thing] + WS_Comet_Blue_offset];

    // @todo check for unreasonable tail lengths?

    // Let's move the head, updating state.
    int new_head_pos = WS_Comet_state[thing].position; // Need an int in case things go negative.
    new_head_pos += WS_Comet_state[thing].direction ? 1 : -1;

    // Check bounds, and bounce if needed, while updating state.
    if (new_head_pos < 0)
    {
        new_head_pos = 0;
        WS_Comet_state[thing].direction = !WS_Comet_state[thing].direction;
    }
    else if (new_head_pos >= thing_led_count[thing])
    {
        new_head_pos = thing_led_count[thing] - 1;
        WS_Comet_state[thing].direction = !WS_Comet_state[thing].direction;
    }

    // Save new_head_pos.
    WS_Comet_state[thing].position = new_head_pos;

    // printf("WS comet thing %d, new head x-pos %d, new head dir %d\n", thing, WS_Comet_state[thing].position, WS_Comet_state[thing].direction); // @debug

    // Start at comet head. "X" as in on an axis (Vs Y, etc).
    int curr_pos_in_comet = 1;
    int curr_body_x_pos = WS_Comet_state[thing].position;
    // Negate so cursor runs opposite of head direction to tail.
    uint8_t cursor_direction = !WS_Comet_state[thing].direction;

    // Iterate from head to tail to find tail pos.
    while (curr_pos_in_comet <= registry[reg_thing_start[thing] + WS_Comet_Tail_offset])
    {
        // Figure out next body position.
        curr_body_x_pos += cursor_direction ? 1 : -1;
        // Respect bounds of Thing.
        if (curr_body_x_pos < 0)
        {
            curr_body_x_pos = 0;
            cursor_direction = !cursor_direction;
        }
        else if (curr_body_x_pos >= thing_led_count[thing])
        {
            curr_body_x_pos = thing_led_count[thing] - 1;
            cursor_direction = !cursor_direction;
        }

        curr_pos_in_comet++;

        // @note Now we know the position and direction of the tail:
        // tail_direction = !cursor_direction; tail_position = curr_body_pos;

        // @debug explain! Perhaps because last loop this var was head to tail, now it's opposite.
        uint8_t body_dir = !cursor_direction;
        
        // Iterate from end of comet tail (length) to comet head 1 (1's based)
        // ... and populate alpha layer.
        // @note curr_body_pos (tail) left over from last loop!
        for (
            int curr_pos_in_comet = registry[reg_thing_start[thing] + WS_Comet_Tail_offset]; // 1 based.
            curr_pos_in_comet > 0;
            curr_pos_in_comet--
            )
        {
            alpha_layer_leds[curr_body_x_pos].color[0] = curr_led_r;
            alpha_layer_leds[curr_body_x_pos].color[1] = curr_led_g;
            alpha_layer_leds[curr_body_x_pos].color[2] = curr_led_b;
            // @todo does below formula make sense? May be backwards, and later TweenHexColors() needs fixing.
            // @todo should also see if alpha_layer_leds[] item already has an alpha and ave/sum them?
            // a0 = aa + ab * (1 - aa). See https://en.wikipedia.org/wiki/Alpha_compositing .
            // ex from lib: int32_t b = (FastMultiply( hab, aamt ) + FastMultiply( hbb, bamt ) + 128) >> 8;
            
            // Calc new alpha.
            // A hopefully faster version of: alpha_layer_leds[curr_body_x_pos].alpha = curr_pos_in_comet * 255 / registry[reg_thing_start[thing] + WS_Comet_Tail_offset];
            uint8_t tail_len = registry[reg_thing_start[thing] + WS_Comet_Tail_offset];
            // Both ops are 1's based, so we don't want the end of the tail to be 0.
            int ab = FastMultiply(255, (tail_len - curr_pos_in_comet) + 1); // (big, small).
            // printf("ab %d, tail_len %d\n", ab, tail_len); // @debug

            // See if we can optimize division.
            if (byteIsPowerOfTwo(tail_len))
            {
                // GCC: This function is used to count the trailing zeros of the given integer. Note : ctz = count trailing zeros. 
                // See https://www.geeksforgeeks.org/builtin-functions-gcc-compiler/ .
                // @todo do I need a equiv to FastMultiply( hbb, bamt ) + 128) >> 8 here to avoid rounding errors?
                ab = ab >> __builtin_ctz(tail_len);
            }
            else
            {
                // Take the non-native div cost hit.
                ab = ab / tail_len;
            }

            // Check for existing alpha for this position.
            if (alpha_layer_leds[curr_body_x_pos].alpha && 0) // @debug disabling this for now...
            {
                // @debug ****** still broken as of 9/27 14:30 *******
                // Get existing alpha.
                int aa = alpha_layer_leds[curr_body_x_pos].alpha;
                // @debug @todo **** this is causing "blinking" when comet overlaps itself. *****
                // int ab = curr_pos_in_comet * 255 / registry[reg_thing_start[thing] + WS_Comet_Tail_offset]; // @todo optimize.
                int a0 = aa + FastMultiply(ab, 255 - aa); // AKA ab * (255 - aa);
                // @debug still not working well.
                alpha_layer_leds[curr_body_x_pos].alpha = a0 > 255 ? 255 : a0; // @debug testing...
                // printf("has alpha: aa %d, ab %d, alpha %d\n", aa, ab, alpha_layer_leds[curr_body_x_pos].alpha); // @debug
            }
            else
            {
                alpha_layer_leds[curr_body_x_pos].alpha = ab;
                // printf("no alpha: ab %d, alpha %d\n", ab, alpha_layer_leds[curr_body_x_pos].alpha); // @debug
            }
            // @debug original, working: alpha_layer_leds[curr_body_x_pos].alpha = curr_pos_in_comet * 255 / registry[reg_thing_start[thing] + WS_Comet_Tail_offset];
            
            // Mark as used.
            alpha_layer_leds[curr_body_x_pos].mask = 1;

            // Figure out next body position.
            curr_body_x_pos += body_dir ? 1 : -1;
            // Respect bounds of Thing.
            if (curr_body_x_pos < 0)
            {
                curr_body_x_pos = 0;
                body_dir = !body_dir;
            }
            else if (curr_body_x_pos >= thing_led_count[thing])
            {
                curr_body_x_pos = thing_led_count[thing] - 1; // 0 based.
                body_dir = !body_dir;
            }
        }

        // Loop through Thing's reg LEDs and then apply alpha layer calc to each.
        for (int i = 0; i < thing_led_count[thing]; i++)
        {
            // If masked, just set to background color.
            if (alpha_layer_leds[i].mask == 0)
                {
                    // Background it is!
                    // Output to registry LEDs.
                    for (int j = 0; j < 3; j++)
                    {
                        registry[reg_thing_led_start[thing] + (i * 3) + j] = registry[reg_thing_start[thing] + WS_Comet_BG_color_offset + j];
                    }
                    // Short circuit the rest of the loop.
                    continue;
                }

            // Apply alpha layer.
            // @note see also https://stackoverflow.com/questions/726549/algorithm-for-additive-color-mixing-for-rgb-values .
            uint32_t hexa = alpha_layer_leds[i].color[0] << 16 | alpha_layer_leds[i].color[1] << 8 | alpha_layer_leds[i].color[2];
            uint32_t hexb = registry[reg_thing_start[thing] + WS_Comet_BG_color_offset] << 16 | registry[reg_thing_start[thing] + WS_Comet_BG_color_offset + 1] << 8 | registry[reg_thing_start[thing] + WS_Comet_BG_color_offset + 2];
            // @note alpha applies to the 2nd param (hexa) in this function.
            uint32_t hex_color = TweenHexColors(hexb, hexa, alpha_layer_leds[i].alpha);
            
            uint8_t r = (hex_color >> 16) & 0xFF;
            uint8_t g = (hex_color >> 8) & 0xFF;
            uint8_t b = hex_color & 0xFF;

            registry[reg_thing_led_start[thing] + (i * 3) + 0] = r;
            registry[reg_thing_led_start[thing] + (i * 3) + 1] = g;
            registry[reg_thing_led_start[thing] + (i * 3) + 2] = b;
        }
    }

    thing_tock_timer[thing] = registry[reg_thing_start[thing] + WS_Comet_Timer_offset] * 256 + registry[reg_thing_start[thing] + WS_Comet_Timer_offset + 1];

    return 1; // @note Routine is always dirty.... for now.
}
