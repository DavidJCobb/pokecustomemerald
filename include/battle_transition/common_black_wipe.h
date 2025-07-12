#ifndef GUARD_BATTLE_TRANSITION_COMMON_BLACK_WIPE_H
#define GUARD_BATTLE_TRANSITION_COMMON_BLACK_WIPE_H

#include "gba/types.h"

// These use data[0] through data[10].
extern void BattleTransitionCommon_InitBlackWipe(s16* data, s16 startX, s16 startY, s16 endX, s16 endY, s16 xMove, s16 yMove);
extern bool8 BattleTransitionCommon_UpdateBlackWipe(s16*, bool8 xExact, bool8 yExact);

// Some transitions that use black wipes check the wipe state on their own, and as such 
// need to be able to see these field names.
#define tWipeStartX data[0]
#define tWipeStartY data[1]
#define tWipeCurrX  data[2]
#define tWipeCurrY  data[3]
#define tWipeEndX   data[4]
#define tWipeEndY   data[5]
#define tWipeXMove  data[6]
#define tWipeYMove  data[7]
#define tWipeXDist  data[8]
#define tWipeYDist  data[9]
#define tWipeTemp   data[10]

#endif