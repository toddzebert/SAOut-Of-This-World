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
    0, // Timer_default_H = 0;
    150, // Timer_default_L = 15
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

struct {
    uint8_t head_pos_idx: 4; // 0-15 but we only need max 9, in WS_Comet_pos_data[].
    uint8_t tail_pos_idx: 4; // 0-15 but we only need max 9, in WS_Comet_pos_data[].
    uint8_t direction: 1; // 0 = left, 1 = right.
    uint8_t status: 2; // 0-3 - use will depend on the mode. @todo use is TBD.
    uint8_t tail_length: 5; // 16 max, but 1-based. // @todo might be reduced.
} WS_Comet_state[THING_COUNT];

struct Alpha_Layer_LED_t {
    uint8_t color[3];
    uint8_t alpha; // W/o FP this is a 0-7, for shifting.
    // @debug remove uint8_t mask; // @todo needed?
};

int effect_ws_comet_run(Things_t thing, Event_t event);

// @todo simplify this.
struct {
    uint8_t led_positon: 4; // 0-15 but we only need max 9.
    // @debug remove uint8_t mask: 5; // need up 1-16 // @todo might be reduced.
} WS_Comet_Pos_data[THING_COUNT][16]; // @todo make max length a define and replace everywhere.

// @todo make it "Alpha" and shouldn't it have be diff for each thing?
uint8_t WS_Comet_alpha_data[16];

int effect_ws_comet(Things_t thing, Event_t event)
{
    switch (event.type)
    {
        case EVENT_INIT:
            // Copy defaults to registry.
            constToRegCopy(registry, reg_thing_start[thing], ws_comet_defaults, 0, sizeof(ws_comet_defaults) * sizeof(uint8_t));

            state_action[thing] = STATE_ACTION_ENTER;
            thing_tock_timer[thing] = 10; // Get this moving to Run soon.

            return 0;

        case EVENT_RUN:
            return effect_ws_comet_run(thing, event);

        case EVENT_REG_CHANGE:
            // @todo
        default:
            return 0;
    }
}

int effect_ws_comet_run(Things_t thing, Event_t event)
{
    int WS_Comet_Head_idx = 0;

    if (state_action[thing] == STATE_ACTION_ENTER)
    {
        // Set initial state. //
        WS_Comet_state[thing].head_pos_idx = 0; // If changed, then the "Populate LED position and Alphas" needs to be changed.
        WS_Comet_state[thing].direction = rnd_fun(0, 1) & 0x01; // To get 0|1.
        WS_Comet_state[thing].status = 0;
        // Limit length to 8. It's 8 so even on Upper Trim, the comet only wraps once.
        WS_Comet_state[thing].tail_length = (registry[reg_thing_start[thing] + WS_Comet_Tail_offset] < 8) ? registry[reg_thing_start[thing] + WS_Comet_Tail_offset] : 8;
        // printf("WS action_enter, comet state: thing %d, pos %d, dir %d, status %d, tail len %d\n",
        //      thing, WS_Comet_state[thing].head_pos_idx, WS_Comet_state[thing].direction, WS_Comet_state[thing].status, WS_Comet_state[thing].tail_length); // @debug

        // Apply background color to registry LEDs. //
        for (int i = 0; i < thing_led_count[thing]; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                registry[reg_thing_led_start[thing] + (i * 3) + j] = registry[reg_thing_start[thing] + WS_Comet_BG_color_offset + j];
            }
        }

        // Populate initial WS_Comet_Pos_data. //
        int temp_led_pos = thing_led_count[thing] >> 1; // Start at middle.
        int temp_next_led_pos;
        // @note this is inverted because next loop goes from head to tail, while GO loop find next head pos.
        uint8_t temp_dir = !WS_Comet_state[thing].direction;

        // Populate LED position and Alphas //
        // Iterate from head to tail.
        for (int i = 0; i < WS_Comet_state[thing].tail_length; i++)
        {
            // Calculate head positions. //
            WS_Comet_Pos_data[thing][i].led_positon = temp_led_pos;
            temp_next_led_pos = temp_led_pos + (temp_dir ? 1 : -1); // +1 = right : -1 = left.

            if (temp_next_led_pos < 0)
            {
                temp_next_led_pos = 1;
                temp_dir = !temp_dir;
            }
            else if (temp_next_led_pos >= thing_led_count[thing])
            {
                temp_next_led_pos = thing_led_count[thing] - 1;
                temp_dir = !temp_dir;
            }

            temp_led_pos = temp_next_led_pos;

            // Set tail when on last iteration.
            if (i == 0) WS_Comet_state[thing].head_pos_idx = temp_led_pos;

            // printf("post calc head pos: i %d, temp_dir %d, data led pos %d\n",
            //      i, temp_dir, WS_Comet_Pos_data[thing][i].led_positon); // @debug

            // Pre-calc alphas. //
            // Replacing alpha = (curr_pos_in_comet * 255) / registry[reg_thing_start[thing] + WS_Comet_Tail_offset];
            // Multiply first. Tail length is 1-based, i is 0-based.
            int partial_eq = FastMultiply(255, WS_Comet_state[thing].tail_length - i); // (big, small).
            // See if we can optimize division, for tail lengths multiples of 2.
            if (byteIsPowerOfTwo(WS_Comet_state[thing].tail_length))
            {
                // GCC: This function is used to count the trailing zeros of the given integer. Note : ctz = count trailing zeros. 
                // See https://www.geeksforgeeks.org/builtin-functions-gcc-compiler/ .
                WS_Comet_alpha_data[i] = partial_eq >> __builtin_ctz(WS_Comet_state[thing].tail_length);
            }
            else
            {
                // Take the non-native-FP div cost hit.
                WS_Comet_alpha_data[i] = WS_Comet_state[thing].tail_length / WS_Comet_state[thing].tail_length;
            }

            // printf("post calc alpha: i %d, alpha %d\n", i, WS_Comet_alpha_data[i]); // @debug
        }

        state_action[thing] = STATE_ACTION_GO;
        thing_tock_timer[thing] = 10; // Come back soon.

        return 1; // Dirty.
    }

    // Assuming STATE_ACTION_GO. //

    // Let's reset/overwrite the current comet's LEDs with background color. //
    WS_Comet_Head_idx = WS_Comet_state[thing].head_pos_idx;

    for (int i = 0; i < WS_Comet_state[thing].tail_length; i++)
    {
        for (int j = 0; j < 3; j++)
            registry[reg_thing_led_start[thing] + (WS_Comet_Pos_data[thing][WS_Comet_Head_idx].led_positon * 3) + j] =
                registry[reg_thing_start[thing] + WS_Comet_BG_color_offset + j];

        WS_Comet_Head_idx = WS_Comet_Head_idx + 1;

        if (WS_Comet_Head_idx >= thing_led_count[thing]) WS_Comet_Head_idx = 0;
    }

    // Temp head LED position.
    // @note has to be done before head_pos_idx changes.
    int Head_LED_pos = WS_Comet_Pos_data[thing][WS_Comet_state[thing].head_pos_idx].led_positon;

    // Reset var, Handle Head pos idx change. //
    WS_Comet_Head_idx = WS_Comet_state[thing].head_pos_idx - 1;

    // Wrap Head idx around.
    // @note see "Wrap temp pos idx around."
    if (WS_Comet_Head_idx < 0) WS_Comet_Head_idx = thing_led_count[thing] - 1;

    // Update state with new Head pos idx.
    WS_Comet_state[thing].head_pos_idx = WS_Comet_Head_idx;

    // Find new LED position of head. //
    Head_LED_pos = Head_LED_pos + (WS_Comet_state[thing].direction ? +1 : -1);

    // Check LED position constraints.
    if (Head_LED_pos < 0)
    {
        Head_LED_pos = 1;
        WS_Comet_state[thing].direction = !WS_Comet_state[thing].direction;
    }
    else if (Head_LED_pos > ((thing_led_count[thing]) - 1))
    {
        Head_LED_pos = thing_led_count[thing] - 1;
        WS_Comet_state[thing].direction = !WS_Comet_state[thing].direction;
    }

    // Save new head LED position.
    WS_Comet_Pos_data[thing][WS_Comet_Head_idx].led_positon = Head_LED_pos;

    // Temp pos idx, starting from head.
    int temp_pos_idx = WS_Comet_state[thing].head_pos_idx;

    // printf("pre-loop: comet head idx %d, head LED pos %d, temp pos idx %d, dir %d\n",
    //     WS_Comet_Head_idx, Head_LED_pos, temp_pos_idx, WS_Comet_state[thing].direction); // @debug

    // Do alpha and Render back to registry. //
    for (int i = 0; i < WS_Comet_state[thing].tail_length; i++)
    {
        // Blend background with foreground with alpha.
        // @todo do this once and save early, although if background is changing...
        uint32_t rgb = blendHexColorsWithAlpha(
            registry[reg_thing_start[thing] + WS_Comet_BG_Red_offset],
            registry[reg_thing_start[thing] + WS_Comet_BG_Green_offset],
            registry[reg_thing_start[thing] + WS_Comet_BG_Blue_offset],
            registry[reg_thing_start[thing] + WS_Comet_Red_offset],
            registry[reg_thing_start[thing] + WS_Comet_Green_offset],
            registry[reg_thing_start[thing] + WS_Comet_Blue_offset],
            WS_Comet_alpha_data[i]
        );

        // Unpack colors.
        uint8_t r = (rgb >> 16) & 0xFF;
        uint8_t g = (rgb >> 8) & 0xFF;
        uint8_t b = rgb & 0xFF;

        // Update registry.
        registry[reg_thing_led_start[thing] + (WS_Comet_Pos_data[thing][temp_pos_idx].led_positon * 3) + 0] = r;
        registry[reg_thing_led_start[thing] + (WS_Comet_Pos_data[thing][temp_pos_idx].led_positon * 3) + 1] = g;
        registry[reg_thing_led_start[thing] + (WS_Comet_Pos_data[thing][temp_pos_idx].led_positon * 3) + 2] = b;

        // Get next part's idx (moving from head to tail).
        temp_pos_idx++;

        // Wrap temp pos idx around.
        // @note see "Wrap Head idx around."
        if (temp_pos_idx == thing_led_count[thing]) temp_pos_idx = 0;
    }

    // Update pos idx state.
    WS_Comet_state[thing].head_pos_idx = WS_Comet_Head_idx;

    thing_tock_timer[thing] = registry[reg_thing_start[thing] + WS_Comet_Timer_offset] * 256 + registry[reg_thing_start[thing] + WS_Comet_Timer_offset + 1];

    return 1; // @note Routine is always dirty.... for now.
}
