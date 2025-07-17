#ifndef GUARD_LU_NAMING_SCREEN_H
#define GUARD_LU_NAMING_SCREEN_H

#include "gba/types.h"

struct LuNamingScreenParams {
   bool8 is_for_pokemon;
   struct {
      u8  gender;
      u16 species;
      u32 personality;
   } pokemon;
   u8 max_length;
};

extern void LuNamingScreen(
   const struct LuNamingScreenParams*
);

#endif