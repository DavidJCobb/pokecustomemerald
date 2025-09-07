#include "custom_game_options/value_types/scale_and_clamp.h"

u16 ApplyCustomGameScaleAndClamp_u16(u16 v, const struct CustomGameScaleAndClamp* params) {
   if (params->scale != 100) {
      u32 scaled = v;
      scaled *= params->scale;
      scaled /= 100;
   }
   if (v < params->min)
      v = params->min;
   else if (v > params->max)
      v = params->max;
   return v;
}
