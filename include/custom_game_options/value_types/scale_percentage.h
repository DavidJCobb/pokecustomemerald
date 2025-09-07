#ifndef GUARD_CGO_VALUETYPE_SCALE_PERCENTAGE
#define GUARD_CGO_VALUETYPE_SCALE_PERCENTAGE

#include "lu/bitpack_options.h"
#include "gba/types.h"

LU_BP_MINMAX(0, 5000) typedef u16 CustomGameScalePct; // 100 = 100%

extern u8  ApplyCustomGameScale_u8 (u8,  CustomGameScalePct scale);
extern u16 ApplyCustomGameScale_u16(u16, CustomGameScalePct scale);
extern u32 ApplyCustomGameScale_u32(u32, CustomGameScalePct scale);
extern s32 ApplyCustomGameScale_s32(s32, CustomGameScalePct scale);

#endif