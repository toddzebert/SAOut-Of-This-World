#include "global.h"

/// #include <stdio.h>

// #include <ch32v003fun.h> // @debug

// enum Things Things_t;

volatile uint8_t registry[128] = {0};

uint32_t ws_leds[16] = {0};

// @todo move this!
void effect_raw(void)
{
    printf("In effect_raw\r\n");
    // @todo
}


// @todo problem below!! ******
// > src\global.c:17:15: warning: initialization discards 'volatile' qualifier from pointer target type [-Wdiscarded-qualifiers]
// > char *d = dest;

// @note "void *" is a pointer to data of unknown type, and cannot be dereferenced.
void regCopy(volatile uint8_t *dest, size_t dest_offset, uint8_t *src, size_t src_offset, size_t dest_len) {
    // @todo
    // char *d = dest;
    const uint8_t *s = src;

    for (size_t i = 0; i < dest_len; i++) {
        dest[dest_offset + i] = s[src_offset + i];
    }

    // > warning: return discards 'volatile' qualifier from pointer target type [-Wdiscarded-qualifiers].
    // return dest;
}
