#include "effects/raw.h"

#define Raw_Timer_offset 1 // 2 bytes

// @debug untested.

int effect_raw(Things_t thing, Event_t event) {
    // @todo ?

    thing_tock_timer[thing] = registry[reg_thing_start[thing] + Raw_Timer_offset] * 256 + registry[reg_thing_start[thing] + Raw_Timer_offset + 1];

    return 1; // Assumed, dunno what the user is changing in the registry.
}
