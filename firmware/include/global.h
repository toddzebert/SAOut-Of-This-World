#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>

// #pragma message ("In global.h") // @debug

#define THING_COUNT 4

// Things.
typedef enum {
    THING_STARS = 0,
    THING_EYES = 1,
    THING_UPPER_TRIM = 2,
    THING_LOWER_TRIM = 3
} Things_t;

extern Things_t thing;

typedef enum {
    STATE_ACTION_INIT, // @todo needed?
    STATE_ACTION_ENTER,
    STATE_ACTION_GO,
    STATE_ACTION_EXIT
} State_Action_t;

typedef enum {
    EVENT_INIT,
    EVENT_RUN,
    EVENT_REG_CHANGE,
    EVENT_BUTTON,
} Event_Type_t;

// @debug probably unneeded.
typedef struct {
    Event_Type_t type;
} Event_Init_t;

// @debug probably unneeded.
typedef struct {
    Event_Type_t type;
} Event_Run_t;

typedef struct {
    Event_Type_t type;
    uint8_t reg;
    uint8_t length;
} Event_Reg_Change_t;

typedef union {
    // Event_Init_t init;
    // Event_Run_t run;
    Event_Reg_Change_t reg_change;
    // @todo button.
} Event_Data_t;

typedef struct {
    Event_Type_t type;
    Event_Data_t data;
} Event_t;

// @debug below is wrong - each thing and effect will have own state phase.
// extern State_Action_t thing_state_phase[THING_COUNT];

#define STARS_COUNT 5 // (white LEDs via GPIO)
#define EYES_COUNT 2 // (WS2812B)
#define UPPER_TRIM_COUNT 5 // (WS2812B)
#define LOWER_TRIM_COUNT 9 // (WS2812B)

extern const uint8_t thing_led_count[];

extern uint16_t thing_timer[THING_COUNT];

extern const uint8_t RGB_Black[3];

#define STARS_GPIO_PINS_NUM 3

// Effects.
#define EFFECT_UNDEFINED 0
#define EFFECT_RAW 1
#define EFFECT_WS_COMET 2
#define EFFECT_WS_BLINK 3
#define EFFECT_TWINKLE 4
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

void constToRegCopy(volatile uint8_t *dest, size_t dest_offset, const uint8_t *src, size_t src_offset, size_t dest_len);

void regToRegCopy(volatile uint8_t *dest, size_t dest_offset, volatile uint8_t *src, size_t src_offset, size_t dest_len);

// Misc functions.
uint8_t byteIsPowerOfTwo(uint8_t x);

// Debug functions.

// Ex: printNon0Reg(registry);
void printNon0Reg(volatile uint8_t *reg);

// EX: printf("Blink state: "); printBin(blink_state, 1);
void printBin(uint8_t c, int newline);

// Ex: printf("Blink state: "); printBinByRef(&Blink_state, 1);
void printBinByRef(void *pnt0, int newline);

#endif /* GLOBAL_H_ */
