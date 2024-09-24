#include "effects/raw.h"

#define Raw_Timer_offset 1 // 2 bytes

int effect_raw(Things_t thing, int flag) {
    // @todo ?

    if (flag == 1) {
        // @todo nothing to do here? Raw values already in place.
    }
    else
    {
        // @todo nothing to do here? Raw values already in place.
    }

    thing_timer[thing] = registry[reg_thing_start[thing] + Raw_Timer_offset] * 256 + registry[reg_thing_start[thing] + Raw_Timer_offset + 1];

    return 1; // Assumed, dunno what the user is changing in the registry.
}