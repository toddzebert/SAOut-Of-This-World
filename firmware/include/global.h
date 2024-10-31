#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>

// #pragma message ("In global.h") // @debug

#define THING_COUNT 6

// Things.
typedef enum {
    THING_STARS,
    THING_EYES,
    THING_UPPER_TRIM,
    THING_LOWER_TRIM,
    THING_BUTTONS,
    THING_GPIOS
} Things_t;

extern Things_t thing;

// The UML State Actions.
typedef enum {
    STATE_ACTION_INIT, // @todo needed?
    STATE_ACTION_ENTER,
    STATE_ACTION_GO,
    STATE_ACTION_EXIT
} State_Action_t;

extern State_Action_t state_action[THING_COUNT];

// The next bunch of statements support Events.
typedef enum {
    EVENT_NONE,
    EVENT_INIT,
    EVENT_RUN,
    EVENT_REG_CHANGE,
    EVENT_BUTTON,
    EVENT_SENSE,
    EVENT_GPIO
} Event_Type_t;

// @debug probably unneeded.
typedef struct {
    uint16_t data; // placeholder
} Event_Init_Data_t;

// @debug probably unneeded.
typedef struct {
    uint16_t data; // placeholder
} Event_Run_Data_t;

// See onI2cWrite().
typedef struct {
    uint8_t reg;
    uint8_t length;
} Event_Reg_Change_Data_t;


typedef enum {
    BUTTON_NONE,
    BUTTON_PRESSED,
    BUTTON_RELEASED,
    BUTTON_LONG_PRESSED,
    BUTTON_DOUBLE_PRESSED
} Button_Event_Type_t;

typedef struct {
    uint8_t num;
    Button_Event_Type_t type;
} Event_Button_Data_t;

typedef union {
    // Event_Init_Data_t init; // not needed...?
    // Event_Run_Data_t run; // not needed...?
    Event_Reg_Change_Data_t reg_change;
    Event_Button_Data_t button;
    // @todo the rest...
} Event_Data_t;

typedef struct {
    Event_Type_t type;
    Event_Data_t data;
} Event_t;

extern Event_t global_event;
extern const Event_t Event_None;

#define STARS_COUNT 5 // (white LEDs via GPIO)
#define EYES_COUNT 2 // (WS2812B)
#define UPPER_TRIM_COUNT 5 // (WS2812B)
#define LOWER_TRIM_COUNT 9 // (WS2812B)

extern const uint8_t thing_led_count[];

extern uint16_t thing_tock_timer[THING_COUNT];

extern const uint8_t RGB_Black[3];

#define STARS_GPIO_H_PINS_NUM 5
#define STARS_GPIO_L_PINS_NUM 2

// Effects.
#define EFFECT_UNDEFINED 0
#define EFFECT_RAW 1
#define EFFECT_WS_COMET 2
#define EFFECT_WS_BLINK 3
#define EFFECT_TWINKLE 4
#define EFFECT_WS_ROTATE 5
// @todo more...

// Registry.
#define REG_STARS_START 32
#define REG_STARS_END 47
#define REG_STARS_COUNT (REG_STARS_END - REG_STARS_START + 1)
#define REG_STARS_LED_START 43

#define REG_EYES_START 48
#define REG_EYES_END 66
#define REG_EYES_COUNT (REG_EYES_END - REG_EYES_START + 1)
#define REG_EYES_LED_START 61
#define REG_EYES_LEFT_LED_START 61
#define REG_EYES_RIGHT_LED_START 64

#define REG_UPPER_TRIM_START 67
#define REG_UPPER_TRIM_END 95
#define REG_UPPER_TRIM_COUNT (REG_UPPER_TRIM_END - REG_UPPER_TRIM_START + 1)
#define REG_UPPER_TRIM_LED_START 81

#define REG_LOWER_TRIM_START 96
#define REG_LOWER_TRIM_END 139
#define REG_LOWER_TRIM_COUNT (REG_LOWER_TRIM_END - REG_LOWER_TRIM_START + 1)
#define REG_LOWER_TRIM_LED_START 113

extern const uint8_t reg_thing_start[THING_COUNT];

extern const uint8_t reg_thing_led_start[THING_COUNT];

// i2c and source of WS callback data.
// @note Can be extended to 256 registers as needed, and presets can be set in the array.
#define REG_COUNT 140
extern volatile uint8_t registry[REG_COUNT];

// Memcpy-like functions.
void arrayToRegCopy(volatile uint8_t *dest, size_t dest_offset, uint8_t *src, size_t src_offset, size_t dest_len);

void regToArrayCopy(uint8_t *dest, size_t dest_offset, volatile uint8_t *src, size_t src_offset, size_t dest_len);

void constToRegCopy(volatile uint8_t *dest, size_t dest_offset, const uint8_t *src, size_t src_offset, size_t dest_len);

void regToRegCopy(volatile uint8_t *dest, size_t dest_offset, volatile uint8_t *src, size_t src_offset, size_t dest_len);

// Misc functions.
uint8_t byteIsPowerOfTwo(uint8_t x);

// Color functions.
// see also color_utilities.h
u_int32_t blendHexColorsWithAlpha(uint8_t br, uint8_t bg, uint8_t bb, uint8_t fr, uint8_t fg, uint8_t fb, uint8_t fa);

// Debug functions.

// Ex: printNon0Reg(registry);
void printNon0Reg(volatile uint8_t *reg);

// EX: printf("Blink state: "); printBin(blink_state, 1);
void printBin(uint8_t c, int newline);

// Ex: printf("Blink state: "); printBinByRef(&Blink_state, 1);
void printBinByRef(void *pnt0, int newline);

#endif /* GLOBAL_H_ */
