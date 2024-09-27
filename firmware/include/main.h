#ifndef MAIN_H_
#define MAIN_H_

#include <ch32v003fun.h>

#include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <stdint.h>

#include <i2c_slave.h>

#include "global.h"
#include "ws2812.h"
#include "button.h"
// #include "effects.h" // @note including breaks everything ;)
#include "things.h"

// ATRLR 10(-1) and PSC 4800(-1) should result in a 1ms timer.
#define SOTW_ATRLR 10
#define SOTW_PSC 4800

#endif /* MAIN_H_ */
