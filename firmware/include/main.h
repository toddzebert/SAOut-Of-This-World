#ifndef MAIN_H_
#define MAIN_H_

#include <ch32v003fun.h>

#include <stdio.h>
#include <string.h> // for memset, more?
// #include <stdlib.h>
// #include <stdint.h>


// @todo document below.
#define DMALEDS 20 // For lib, for testing.

// @todo document below.
#define WS2812DMA_IMPLEMENTATION
#define WSRBG

#include <ch32v003_GPIO_branchless.h>
#include <ws2812b_dma_spi_led_driver.h>
#include <i2c_slave.h>

#include "global.h"
#include "button.h"
// #include "effects.h"
#include "things.h"

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

/* Function declarations */


#endif /* MAIN_H_ */