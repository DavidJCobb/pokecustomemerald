#ifndef GUARD_BATTLE_TRANSITION_COMMON_H
#define GUARD_BATTLE_TRANSITION_COMMON_H

#include "gba/types.h"

struct Task;

typedef bool8 (*TransitionStateFunc)(struct Task *task);

// Common task data
#define tState data[0]

struct BattleTransitionData {
   vu8 VBlank_DMA;
   u16 WININ;
   u16 WINOUT;
   u16 WIN0H;
   u16 WIN0V;
   u16 unused1;
   u16 unused2;
   u16 BLDCNT;
   u16 BLDALPHA;
   u16 BLDY;
   s16 cameraX;
   s16 cameraY;
   s16 BG0HOFS_Lower;
   s16 BG0HOFS_Upper;
   s16 BG0VOFS; // used but not set
   s16 unused3;
   s16 counter;
   s16 unused4;
   s16 data[11];
};

extern struct BattleTransitionData* gBattleTransitionData;

#define with_vblank_dma_disabled \
   for (gBattleTransitionData->VBlank_DMA = FALSE; gBattleTransitionData->VBlank_DMA == 0; ++gBattleTransitionData->VBlank_DMA)

// Needed in vblank callback functions.
#define B_TRANS_DMA_FLAGS (1 | ((DMA_SRC_INC | DMA_DEST_FIXED | DMA_REPEAT | DMA_16BIT | DMA_START_HBLANK | DMA_ENABLE) << 16))

extern void BattleTransitionCommon_InitTransitionData(void);

extern void BattleTransitionCommon_GetBg0TilesDst(u16** tilemap, u16** tileset);
extern void BattleTransitionCommon_GetBg0TilemapDst(u16** tilemap);

extern void BattleTransitionCommon_FadeScreenBlack(void);
extern void BattleTransitionCommon_SetCircularMask(u16* buffer, s16 centerX, s16 centerY, s16 radius);
extern void BattleTransitionCommon_SetSinWave(
   s16* array,
   s16  sinAdd,
   s16  index,
   s16  indexIncrementer,
   s16  amplitude,
   s16  arrSize
);
extern void BattleTransitionCommon_VBlankCB(void); // vanilla: VBlankCB_BattleTransition

#endif