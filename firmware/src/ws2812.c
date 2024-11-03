#include "ws2812.h"

// Internal use.
#define LEDS 16
// @note due to quirks of the WS lib, the below needs to be divisible by 4, AND the first 2 are "dead", so pad it.
#define DMALEDS 20

#define WS2812DMA_IMPLEMENTATION
#define WSRBG

#include <ws2812b_dma_spi_led_driver.h>

/*
WS2812B:
    Send data at speeds of 800Kbps.
*/

// @todo find source....
const uint8_t gamma8[]  = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1, // 16+
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2, // 32+
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5, // 48+
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10, // 64+
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

// @note [move this comment] green is brightest, followed closely by red, then a distant blue.
const uint8_t reg_reserved_ws[] = {
    1, // gamma adjust, default 1
    1, // blue compensation, default 1
    0, // red compensation, default 0 
};

#define REG_DEFAULTS_START 16
#define REG_DEFAULTS_NUM 3

#define REG_GAMMA 16
#define REG_BLUE_COMPENSATION 17
#define REG_RED_COMPENSATION 18 // @todo not sure how to do this without FP. 

// Where the WS callback will gets its data.
uint32_t leds[LEDS] = {0};

/**
 * @brief Initializes the WS2812 LED system.
 *
 * This function sets up the WS2812 LED system by initializing the DMA settings
 * for the specified number of LEDs. It also starts the DMA process to handle
 * any initial LED configurations and copies default registry values from a
 * predefined array to the registry.
 */
void WS2812_Init()
{
    WS2812BDMAInit(LEDS);
    // Get out any LED stuff initial Things have set.
    WS2812BDMAStart(LEDS);

    // Copy defaults to registry.
    constToRegCopy(registry, REG_DEFAULTS_START, reg_reserved_ws, 0, sizeof(reg_reserved_ws) * sizeof(uint8_t));
}


/**
 * @brief Callback function to retrieve the color for a specific LED.
 * 
 * This function is called to get the color data for a specified LED
 * number. The color format is in 0xRRGGBB.
 *
 * @param ledno The index of the LED for which to retrieve the color.
 * @return The color value of the LED as a 32-bit integer.
 */
uint32_t WS2812BLEDCallback( int ledno )
{
    return leds[ledno];
}


uint32_t CalcWSLed(int ledno)
{
    uint8_t r, g, b;
    uint32_t color;

    int index = 0;

    // @debug maybe avoid this conditional and *3's and just make a [16] lookup table?
    // Eyes.
    if (ledno < EYES_COUNT) {
        index = REG_EYES_LED_START + (ledno * 3);
    }
    // Upper trim.
    else if (ledno < (EYES_COUNT + UPPER_TRIM_COUNT)) {
        index = REG_UPPER_TRIM_LED_START + (ledno - EYES_COUNT) * 3;
    }
    // Lower trim.
    else
    {
        index = REG_LOWER_TRIM_LED_START + (ledno - EYES_COUNT - UPPER_TRIM_COUNT) * 3;
    }

    r = registry[index];
    g = registry[index + 1];
    b = registry[index + 2];

    if (registry[REG_BLUE_COMPENSATION] == 1) {
        // Let's reduce them without needing FP.
        // @todo this needs to be revisited.
        r = (r >> 1) + (r >> 2) + (r >> 3); // 87.5%.
        g = (g >> 1) + (g >> 2) + (g >> 3);
        // printf("r %u g %u post blue compensation\n", r, g); // @debug
    }

    if (registry[REG_GAMMA] == 1) {
        r = gamma8[r];
        g = gamma8[g];
        b = gamma8[b];
    }

    // Wrap it up in a uint32_t.
    color = (r << 16) | (g << 8) | b;

    // if (color) { printf("WS led %d idx %d..+2 has color\n", ledno, index); } // @debug
    // printf("WS2812BLEDCallback: ledno %d reg index %d raw_color 0x%08lx\n", ledno, index, color); // @debug

    return color;
}


/**
 * @brief Interrupt handler for WS2812 LED calculations.
 *
 * This function is called from Timer3 [*needs verification] interrupt handler and is responsible for
 * calculating the color for each LED and calling the WS2812BDMAStart() function
 * to start the DMA transfer of the calculated colors to the LEDs.
 */
void WS2812_Handler()
{
    // Iterate through the LEDs and then save the calculated colors.
    for (int i = 0; i < LEDS; i++)
    {
        leds[i] = CalcWSLed(i);
    }

    // Alert the lib we're ready for it's callbacks.
    WS2812BDMAStart(LEDS);
}
