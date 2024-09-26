#include "ws2812.h"

// Internal use.
#define LEDS 16
// This is default in the lib but making it explicit.
#define DMALEDS 16
#define WS2812DMA_IMPLEMENTATION
#define WSRBG // @debug VS RGB? *****

#include <ws2812b_dma_spi_led_driver.h>

/*
WS2812B:
    Send data at speeds of 800Kbps.
*/

// Where the WS callback will gets its data.
uint32_t leds[LEDS] = {0};

void WS2812_Init()
{
    WS2812BDMAInit(LEDS);
    // Get out any LED stuff initial Things have set.
    WS2812BDMAStart(LEDS);
}


uint32_t WS2812BLEDCallback( int ledno )
{
    return leds[ledno];
}


uint32_t CalcWSLed(int ledno)
{
    uint32_t color;

    int index = 0;
    // char *thing = ""; // @debug

    // @debug maybe avoid this conditional and *3's and just make a [16] lookup table?
    // @debug ... OR process reg with color correction/etc to a straight array?
    if (ledno < EYES_COUNT) {
        index = REG_EYES_LED_START + (ledno * 3);
        // thing = "eyes"; // @debug
    }
    else if (ledno < (EYES_COUNT + UPPER_TRIM_COUNT)) {
        index = REG_UPPER_TRIM_LED_START + (ledno - EYES_COUNT) * 3;
        // thing = "upper trim"; // @debug
    }
    else
    {
        index = REG_LOWER_TRIM_LED_START + (ledno - EYES_COUNT - UPPER_TRIM_COUNT) * 3;
        // thing = "lower trim"; // @debug
    }

    // The WS2812B lib expects the color in the format 0x00RRGGBB.
    // @todo could also try casting the 3 consecutive bytes into a uint32_t (starting from index-1, and then clearing MSB).
    color = (uint32_t) registry[index] << 16 | (uint32_t) registry[index + 1] << 8 | (uint32_t) registry[index + 2];

    // @todo adjust for blue vs other colors.
    // @todo gamma adjustment.

    // if (color) { printf("WS led %d idx %d\n", ledno, index); } // @debug
    // @note WARNING: the following debugs will cause unpredictable behavior because they take too long.
    // printf("WS2812BLEDCallback: ledno %d thing %s reg index %d raw_color 0x%08lx\n", ledno, thing, index, color); // @debug
    // printf("WS2812BLEDCallback: ledno %d reg index %d raw_color 0x%08lx\n", ledno, index, color); // @debug

    return color;
}


void WS2812_Handler()
{
    // @todo any prep here?

    // Iterate through the LEDs and then save the calculated colors.
    for (int i = 0; i < LEDS; i++)
    {
        leds[i] = CalcWSLed(i);
    }

    // printNon0Reg(registry); // @debug
    /* @debug debug output:
    WS2812BLEDCallback: ledno 15 reg index 139 raw_color 0x00000b00
    32:0x04 34:0x4b 35:0x01 36:0x80 43:0x01 44:0x01 48:0x03 50:0x96 51:0x6a 52:0x5a
    53:0xcd 54:0x7b 55:0x68 56:0xee 57:0x01 58:0x0f 61:0x6a 62:0x5a 63:0xcd 64:0x7b
    65:0x68 66:0xee
    */

    // Alert the lib we're ready for it's callbacks.
    WS2812BDMAStart(LEDS);
}
