#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

/*
    LED sections:
        Stars (not WS2812B)
        Eyes
        Upper Trim
        Lower Trim
*/

// ATRLR 10(-1) and PSC 4800(-1) should result in a 1ms timer.
#define SOTW_ATRLR 10
#define SOTW_PSC 4800

#define WS_EYES_LED_START 0
#define WS_EYES_LED_END 1
#define WS_EYES_LED_COUNT (WS_EYES_LED_END - WS_EYES_LED_START + 1)

#define WS_UPPER_TRIM_START 3 // @debug leaving one blank
#define WS_UPPER_TRIM_END 7
#define WS_UPPER_TRIM_COUNT (WS_UPPER_TRIM_END - WS_UPPER_TRIM_START + 1)

#define WS_LOWER_TRIM_START 9 // @debug leaving one blank
#define WS_LOWER_TRIM_END 16 // @debug short by one
#define WS_LOWER_TRIM_COUNT (WS_LOWER_TRIM_END - WS_LOWER_TRIM_START + 1)


typedef struct WsEffect
{
    char machine_name[16];
    char name[32];
    const uint16_t default_timer_base;
    uint16_t timer_base;
    uint16_t timer_count;
    const uint32_t default_color;
    uint32_t color;
    const uint8_t default_style;
    uint8_t style;
    const uint8_t default_position;
    uint8_t position;
} WsEffect;

typedef struct WsSections
{
    char machine_name[16];
    const uint8_t start;
    const uint8_t end;
    const uint8_t count;
    WsEffect default_effect;
    WsEffect effect;
} WsSections;


/* Function declarations */


#endif /* MAIN_H */