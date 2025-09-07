#include "custom_game_options/value_types/scale_percentage.h"

u8 ApplyCustomGameScale_u8(u8 v, CustomGameScalePct scale) {
   if (scale != 100) {
      v = ((u32)v * scale) / 100;
      if (v > 255)
         v = 255;
   }
   return v;
}
u16 ApplyCustomGameScale_u16(u16 v, CustomGameScalePct scale) {
   if (scale != 100) {
      v = ((u32)v * scale) / 100;
   }
   return v;
}
u32 ApplyCustomGameScale_u32(u32 v, CustomGameScalePct scale) {
   if (scale != 100) {
      v = (v * scale) / 100;
   }
   return v;
}
s32 ApplyCustomGameScale_s32(s32 v, CustomGameScalePct scale) {
   if (scale != 100) {
      v = (v * scale) / 100;
   }
   return v;
}
