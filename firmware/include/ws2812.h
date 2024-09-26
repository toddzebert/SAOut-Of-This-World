#ifndef WS2812_H_
#define WS2812_H_

#include <ch32v003fun.h>

#include <stdio.h>

#include "global.h"
#include "things.h"

void WS2812_Init();

void WS2812_Handler();

uint32_t WS2812BLEDCallback( int ledno );

#endif /* WS2812_H_ */
