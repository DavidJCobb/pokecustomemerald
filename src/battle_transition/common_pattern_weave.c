#include "battle_transition/common.h"
#include "battle_transition/common_pattern_weave.h"
#include "gba/gba.h"
#include "decompress.h"
#include "main.h"
#include "palette.h"
#include "scanline_effect.h"
#include "task.h"

#define SET_TILE(ptr, posY, posX, tile) \
{                                       \
    u32 index = (posY) * 32 + posX;     \
    ptr[index] = tile | (0xF0 << 8);    \
}

static void VBlankCB_SetWinAndBlend(void);
static void VBlankCB_PatternWeave(void);
static void VBlankCB_CircularMask(void);

#define tBlendTarget1 data[1]
#define tBlendTarget2 data[2]
#define tBlendDelay   data[3]

// Data 1-3 change purpose for PatternWeave_CircularMask
#define tRadius      data[1]
#define tRadiusDelta data[2]
#define tVBlankSet   data[3]

#define tSinIndex     data[4]
#define tAmplitude    data[5]
#define tEndDelay     data[8]

static void InitPatternWeaveTransition(struct Task* task);

extern void BattleTransitionCommon_PatternWeave_Exec(
   struct Task* task,
   const struct BattleTransitionPatternWeave* pattern
) {
   u16 *tilemap, *tileset;
   
   bool8 advance = FALSE;
   switch (task->tState) {
      case TASK_STATE_INIT_TILESET:
         {
            task->tEndDelay = pattern->end_delay;
            InitPatternWeaveTransition(task);
            BattleTransitionCommon_GetBg0TilesDst(&tilemap, &tileset);
            CpuFill16(0, tilemap, BG_SCREEN_SIZE);
            if (pattern->tileset.compressed) {
               LZ77UnCompVram(pattern->tileset.data, tileset);
            } else {
               u16 size = pattern->tileset.size;
               if (size == 0)
                  size = 0x2000;
               CpuCopy16(pattern->tileset.data, tileset, size);
            }
            LoadPalette(pattern->palette.data, BG_PLTT_ID(15), pattern->palette.size);
         }
         advance = TRUE;
         break;
      case TASK_STATE_INIT_TILEMAP:
         {
            BattleTransitionCommon_GetBg0TilesDst(&tilemap, &tileset);
            switch (pattern->tilemap.type) {
               case TILEMAP_IS_COMPRESSED:
                  LZ77UnCompVram(pattern->tilemap.data, tilemap);
                  break;
               case TILEMAP_IS_UNCOMPRESSED:
                  {
                     u16 size = pattern->tilemap.size;
                     if (size == 0)
                        size = 0x500;
                     CpuCopy16(pattern->tilemap.data, tilemap, size);
                  }
                  break;
               case TILEMAP_IS_RECT:
                  {
                     const u16* src = pattern->tilemap.data;
                     u8 max_x = pattern->tilemap.rect.x;
                     u8 max_y = pattern->tilemap.rect.y;
                     for (u8 i = 0; i < max_y; i++)
                        for (u8 j = 0; j < max_x; j++, src++)
                           SET_TILE(tilemap, i, j, *src);
                  }
                  break;
            }
            BattleTransitionCommon_SetSinWave(gScanlineEffectRegBuffers[0], 0, task->tSinIndex, 132, task->tAmplitude, DISPLAY_HEIGHT);
         }
         advance = TRUE;
         break;
      case TASK_STATE_BLEND_1:
         with_vblank_dma_disabled {
            if (task->tBlendDelay == 0 || --task->tBlendDelay == 0) {
               task->tBlendTarget2++;
               task->tBlendDelay = 2;
            }
            gBattleTransitionData->BLDALPHA = BLDALPHA_BLEND(task->tBlendTarget2, task->tBlendTarget1);
            if (task->tBlendTarget2 > 15) {
               advance = TRUE;
            }
            task->tSinIndex += 8;
            task->tAmplitude -= 256;

            BattleTransitionCommon_SetSinWave(gScanlineEffectRegBuffers[0], 0, task->tSinIndex, 132, task->tAmplitude >> 8, DISPLAY_HEIGHT);
         }
         break;
      case TASK_STATE_BLEND_2:
         with_vblank_dma_disabled {
            if (task->tBlendDelay == 0 || --task->tBlendDelay == 0) {
               task->tBlendTarget1--;
               task->tBlendDelay = 2;
            }
            gBattleTransitionData->BLDALPHA = BLDALPHA_BLEND(task->tBlendTarget2, task->tBlendTarget1);
            if (task->tBlendTarget1 == 0) {
               advance = TRUE;
            }
            task->tSinIndex  += 8;
            task->tAmplitude -= 256;

            BattleTransitionCommon_SetSinWave(gScanlineEffectRegBuffers[0], 0, task->tSinIndex, 132, task->tAmplitude >> 8, DISPLAY_HEIGHT);
         }
         break;
      case TASK_STATE_FINISH_APPEAR:
         with_vblank_dma_disabled {
            task->tSinIndex  += 8;
            task->tAmplitude -= 256;

            BattleTransitionCommon_SetSinWave(gScanlineEffectRegBuffers[0], 0, task->tSinIndex, 132, task->tAmplitude >> 8, DISPLAY_HEIGHT);

            if (task->tAmplitude <= 0) {
               advance = TRUE;
               task->tRadius      = DISPLAY_HEIGHT;
               task->tRadiusDelta = 1 << 8;
               task->tVBlankSet   = FALSE;
            }
         }
         break;
      case TASK_STATE_END_DELAY:
         if (--task->tEndDelay == 0)
            advance = TRUE;
         break;
      case TASK_STATE_CIRCULAR_MASK:
         // Do a shrinking circular mask to go to a black screen after the pattern appears.
         {
            gBattleTransitionData->VBlank_DMA = FALSE;
            if (task->tRadiusDelta < (4 << 8))
               task->tRadiusDelta += 128; // 256 is 1 unit of speed. Speed up every other frame (128 / 256)
            if (task->tRadius != 0) {
               task->tRadius -= task->tRadiusDelta >> 8;
               if (task->tRadius < 0)
                  task->tRadius = 0;
            }
            BattleTransitionCommon_SetCircularMask(gScanlineEffectRegBuffers[0], DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, task->tRadius);
            if (task->tRadius == 0) {
               SetVBlankCallback(NULL);
               DmaStop(0);
               BattleTransitionCommon_FadeScreenBlack();
               advance = TRUE;
            } else {
               if (!task->tVBlankSet) {
                  task->tVBlankSet++;
                  SetVBlankCallback(VBlankCB_CircularMask);
               }
               gBattleTransitionData->VBlank_DMA++;
            }
         }
         break;
   }
   if (advance) {
      ++task->tState;
      if (task->tState == TASK_STATE_END_DELAY && task->tEndDelay == 0) {
         ++task->tState;
      }
      if (task->tState == TASK_STATE_DONE) {
         DestroyTask(FindTaskIdByFunc(task->func));
      }
   }
}

static void InitPatternWeaveTransition(struct Task* task) {
   s32 i;

   BattleTransitionCommon_InitTransitionData();
   ScanlineEffect_Clear();

   task->tBlendTarget1 = 16;
   task->tBlendTarget2 = 0;
   task->tSinIndex = 0;
   task->tAmplitude = 0x4000;
   gBattleTransitionData->WININ    = WININ_WIN0_ALL;
   gBattleTransitionData->WINOUT   = 0;
   gBattleTransitionData->WIN0H    = DISPLAY_WIDTH;
   gBattleTransitionData->WIN0V    = DISPLAY_HEIGHT;
   gBattleTransitionData->BLDCNT   = BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_ALL;
   gBattleTransitionData->BLDALPHA = BLDALPHA_BLEND(task->tBlendTarget2, task->tBlendTarget1);

   for (i = 0; i < DISPLAY_HEIGHT; i++)
      gScanlineEffectRegBuffers[1][i] = DISPLAY_WIDTH;

   SetVBlankCallback(VBlankCB_PatternWeave);
}

static void VBlankCB_SetWinAndBlend(void) {
   DmaStop(0);
   BattleTransitionCommon_VBlankCB();
   if (gBattleTransitionData->VBlank_DMA)
      DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 2);
   REG_WININ    = gBattleTransitionData->WININ;
   REG_WINOUT   = gBattleTransitionData->WINOUT;
   REG_WIN0V    = gBattleTransitionData->WIN0V;
   REG_BLDCNT   = gBattleTransitionData->BLDCNT;
   REG_BLDALPHA = gBattleTransitionData->BLDALPHA;
}

static void VBlankCB_PatternWeave(void) {
   VBlankCB_SetWinAndBlend();
   DmaSet(0, gScanlineEffectRegBuffers[1], &REG_BG0HOFS, B_TRANS_DMA_FLAGS);
}

static void VBlankCB_CircularMask(void) {
   VBlankCB_SetWinAndBlend();
   DmaSet(0, gScanlineEffectRegBuffers[1], &REG_WIN0H, B_TRANS_DMA_FLAGS);
}

#undef tAmplitude
#undef tSinIndex
#undef tBlendTarget1
#undef tBlendTarget2
#undef tRadius
#undef tRadiusDelta
#undef tVBlankSet