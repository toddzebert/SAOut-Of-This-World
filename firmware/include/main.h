#ifndef MAIN_H_
#define MAIN_H_

// Systick = 48MHz.
#define FUNCONF_SYSTICK_USE_HCLK 1

#include <ch32v003fun.h>

#include <stdio.h>
#include <stdbool.h>
// #include <string.h>
// #include <stdlib.h>
// #include <stdint.h>

// @debug PA: #include <i2c_slave.h>

#include "global.h"
// @debug PA: #include "ws2812.h"
#include "button.h"
#include "things.h"

// Number of ticks elapsed per millisecond (48,000 when using 48MHz Clock)
#define SYSTICK_ONE_MILLISECOND ((uint32_t)FUNCONF_SYSTEM_CORE_CLOCK / 1000)

// Number of ticks elapsed per millisecond (48,000 when using 48MHz Clock)
#define SYSTICK_ONE_TENTH_MILLISECOND ((uint32_t)FUNCONF_SYSTEM_CORE_CLOCK / 10000)

#endif /* MAIN_H_ */
