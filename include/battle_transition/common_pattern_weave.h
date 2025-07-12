#ifndef GUARD_BATTLE_TRANSITION_COMMON_PATTERN_WEAVE_H
#define GUARD_BATTLE_TRANSITION_COMMON_PATTERN_WEAVE_H

#include "gba/defines.h" // TRUE, FALSE
#include "gba/types.h"

enum TilemapType {
   TILEMAP_IS_UNCOMPRESSED,
   TILEMAP_IS_COMPRESSED,
   TILEMAP_IS_RECT,
};

struct BattleTransitionPatternWeave {
   u16 end_delay;
   struct {
      const u16* data;
      u16        size;
   } palette;
   struct {
      const void* data;
      union {
         u16 size;
         struct {
            u8 x;
            u8 y;
         } rect;
      };
      enum TilemapType type;
   } tilemap;
   struct {
      const void* data;
      u16         size;
      bool8       compressed;
   } tileset;
};

enum TASK_STATE {
   TASK_STATE_INIT_TILESET = 0,
   TASK_STATE_INIT_TILEMAP,
   TASK_STATE_BLEND_1,
   TASK_STATE_BLEND_2,
   TASK_STATE_FINISH_APPEAR,
   TASK_STATE_END_DELAY,
   TASK_STATE_CIRCULAR_MASK,
   //
   TASK_STATE_DONE
};

extern void BattleTransitionCommon_PatternWeave_Exec(
   struct Task*,
   const struct BattleTransitionPatternWeave*
);

#endif