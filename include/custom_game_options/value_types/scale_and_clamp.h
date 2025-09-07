#ifndef GUARD_CGO_VALUETYPE_SCALE_AND_CLAMP
#define GUARD_CGO_VALUETYPE_SCALE_AND_CLAMP

#include "lu/bitpack_options.h"
#include "gba/types.h"

#include "./scale_percentage.h"

struct CustomGameScaleAndClamp {
   CustomGameScalePct scale;
   u16 min;
   u16 max; // 0xFFFF = no max
};

extern u16 ApplyCustomGameScaleAndClamp_u16(u16, const struct CustomGameScaleAndClamp*);

#endif