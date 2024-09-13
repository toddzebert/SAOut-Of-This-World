#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
// #include <effects/blink.h>

// Things. @todo delete?
/*
#define THING_STARS 1
#define THING_EYES 2
#define THING_UPPER_TRIM 3
#define THING_LOWER_TRIM 4
*/

// Things.
// /* @debug
typedef enum {
    THING_STARS = 0,
    THING_EYES = 1,
    THING_UPPER_TRIM = 2,
    THING_LOWER_TRIM = 3
} Things_t;

extern Things_t thing;
// */

// #define GLOBAL_DEBUG 1

// WS Things start, end, count.
#define WS_EYES_LED_START 0
#define WS_EYES_LED_END 1
#define WS_EYES_LED_COUNT (WS_EYES_LED_END - WS_EYES_LED_START + 1)

#define WS_UPPER_TRIM_START 3 // @debug leaving one blank
#define WS_UPPER_TRIM_END 7
#define WS_UPPER_TRIM_COUNT (WS_UPPER_TRIM_END - WS_UPPER_TRIM_START + 1)

#define WS_LOWER_TRIM_START 9 // @debug leaving one blank
#define WS_LOWER_TRIM_END 16 // @debug short by one, limitation of test ring.
#define WS_LOWER_TRIM_COUNT (WS_LOWER_TRIM_END - WS_LOWER_TRIM_START + 1)

// @todo consider making RGB(4th byte?) struct here to make it easier to color? \
// @todo this will be part of registry[].
extern uint32_t ws_leds[16];

// @todo include regular LEDs in same array?

// Effects.
#define EFFECT_UNDEFINED 0
#define EFFECT_RAW 1
#define EFFECT_COMET 2
#define EFFECT_BLINK 3
// @todo more...


#define REG_STARS_START 32
#define REG_STARS_END 47
#define REG_STARS_COUNT (REG_STARS_END - REG_STARS_START + 1)
#define REG_EYES_START 48
#define REG_EYES_END 63
#define REG_EYES_COUNT (REG_EYES_END - REG_EYES_START + 1)
#define REG_UPPER_TRIM_START 64
#define REG_UPPER_TRIM_END 95
#define REG_UPPER_TRIM_COUNT (REG_UPPER_TRIM_END - REG_UPPER_TRIM_START + 1)
#define REG_LOWER_TRIM_START 64
#define REG_LOWER_TRIM_END 95
#define REG_LOWER_TRIM_COUNT (REG_LOWER_TRIM_END - REG_LOWER_TRIM_START + 1)

// i2c.
// @note Can be extended to 256 registers as needed, and presets can be set in the array.
extern volatile uint8_t registry[128];

// void *arrayCopy(volatile void *dest, size_t dest_offset, void *src, size_t src_offset, size_t dest_len);
void regCopy(volatile uint8_t *dest, size_t dest_offset, uint8_t *src, size_t src_offset, size_t dest_len);

// #include "effects.h"
// #include "things.h"

#endif /* GLOBAL_H_ */
