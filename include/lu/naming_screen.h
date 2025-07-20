#ifndef GUARD_LU_VUI_NAMINGSCREEN_H
#define GUARD_LU_VUI_NAMINGSCREEN_H

#include "gba/types.h"

struct LuNamingScreenParams {
   void(*callback)(const u8*);
   const u8* initial_value;
   u8 max_length;
};

extern void LuNamingScreen(const struct LuNamingScreenParams*);

#endif