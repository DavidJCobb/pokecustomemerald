#ifndef GUARD_BATTLE_TRANSITION_COMMON_MUGSHOT_H
#define GUARD_BATTLE_TRANSITION_COMMON_MUGSHOT_H

#include "gba/defines.h" // TRUE, FALSE
#include "gba/types.h"

struct BattleTransitionMugshot {
   struct {
      struct {
         const u16* data;
         u16        size;
      } banner_palette;
      struct {
         struct {
            s16 rotation;
            s16 scale;
         } affine; // sMugshotsOpponentRotationScales[id]
         struct {
            s16 x;
            s16 y;
         } coords; // sMugshotsOpponentCoords[id]
         u16 trainer_pic_id; // sMugshotsTrainerPicIDsTable[id]
      } trainer_sprite;
   } opponent;
};

extern void BattleTransitionCommon_Mugshot_Exec(
   struct Task*,
   const struct BattleTransitionMugshot*
);

#endif