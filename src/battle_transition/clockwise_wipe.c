#include "battle_transition/common.h"
#include "battle_transition/common_black_wipe.h"
#include "battle_transition/clockwise_wipe.h"
#include "gba/gba.h"
#include "gpu_regs.h"
#include "main.h" // SetHBlankCallback, SetVBlankCallback
#include "scanline_effect.h" // gScanlineEffectRegBuffers
#include "task.h"

#define SPEED 2 // vanilla: 1

static bool8 ClockwiseWipe_Init(struct Task*);
static bool8 ClockwiseWipe_TopRight(struct Task*);
static bool8 ClockwiseWipe_Right(struct Task*);
static bool8 ClockwiseWipe_Bottom(struct Task*);
static bool8 ClockwiseWipe_Left(struct Task*);
static bool8 ClockwiseWipe_TopLeft(struct Task*);
static bool8 ClockwiseWipe_End(struct Task*);
//
static const TransitionStateFunc sClockwiseWipe_Funcs[] = {
   ClockwiseWipe_Init,
   ClockwiseWipe_TopRight,
   ClockwiseWipe_Right,
   ClockwiseWipe_Bottom,
   ClockwiseWipe_Left,
   ClockwiseWipe_TopLeft,
   ClockwiseWipe_End
};

static void VBlankCB_ClockwiseWipe(void);

#define tSinVal    data[1]
#define tAmplitude data[2]

void BattleTransitionTaskHandler_ClockwiseWipe(u8 taskId) {
   while (sClockwiseWipe_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
   #if SPEED > 1
   if (gTasks[taskId].tState > 1 && gTasks[taskId].tState < 6) {
      for(u8 i = 1; i < SPEED; ++i) {
         while (sClockwiseWipe_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
      }
   }
   #endif
}

static bool8 ClockwiseWipe_Init(struct Task* task) {
   u16 i;

   BattleTransitionCommon_InitTransitionData();
   ScanlineEffect_Clear();

   gBattleTransitionData->WININ = 0;
   gBattleTransitionData->WINOUT = WINOUT_WIN01_ALL;
   gBattleTransitionData->WIN0H = WIN_RANGE(DISPLAY_WIDTH, DISPLAY_WIDTH + 1);
   gBattleTransitionData->WIN0V = DISPLAY_HEIGHT;

   for (i = 0; i < DISPLAY_HEIGHT; i++)
      gScanlineEffectRegBuffers[1][i] = ((DISPLAY_WIDTH + 3) << 8) | (DISPLAY_WIDTH + 4);

   SetVBlankCallback(VBlankCB_ClockwiseWipe);
   gBattleTransitionData->tWipeEndX = DISPLAY_WIDTH / 2;

   task->tState++;
   return TRUE;
}

static bool8 ClockwiseWipe_TopRight(struct Task* task) {
   with_vblank_dma_disabled {
#ifdef UBFIX
      BattleTransitionCommon_InitBlackWipe(
         gBattleTransitionData->data,
         DISPLAY_WIDTH  / 2,
         DISPLAY_HEIGHT / 2,
         gBattleTransitionData->tWipeEndX,
         0,
         1,
         1
      );
#else
      BattleTransitionCommon_InitBlackWipe(
         gBattleTransitionData->data,
         DISPLAY_WIDTH  / 2,
         DISPLAY_HEIGHT / 2,
         gBattleTransitionData->tWipeEndX,
         -1,
         1,
         1
      );
#endif
      do {
         gScanlineEffectRegBuffers[0][gBattleTransitionData->tWipeCurrY] = (gBattleTransitionData->tWipeCurrX + 1) | ((DISPLAY_WIDTH / 2) << 8);
      } while (!BattleTransitionCommon_UpdateBlackWipe(gBattleTransitionData->data, TRUE, TRUE));

      gBattleTransitionData->tWipeEndX += 16;
      if (gBattleTransitionData->tWipeEndX >= DISPLAY_WIDTH) {
         gBattleTransitionData->tWipeEndY = 0;
         task->tState++;
      }
   }
   return FALSE;
}

static bool8 ClockwiseWipe_Right(struct Task* task) {
   vu8 finished = FALSE;

   with_vblank_dma_disabled {
      BattleTransitionCommon_InitBlackWipe(
         gBattleTransitionData->data,
         DISPLAY_WIDTH  / 2,
         DISPLAY_HEIGHT / 2,
         DISPLAY_WIDTH,
         gBattleTransitionData->tWipeEndY,
         1,
         1
      );

      s16 start;
      s16 end;
      while (1) {
         start = DISPLAY_WIDTH / 2;
         end   = gBattleTransitionData->tWipeCurrX + 1;
         if (gBattleTransitionData->tWipeEndY >= DISPLAY_HEIGHT / 2) {
            start = gBattleTransitionData->tWipeCurrX;
            end   = DISPLAY_WIDTH;
         }
         gScanlineEffectRegBuffers[0][gBattleTransitionData->tWipeCurrY] = end | (start << 8);
         if (finished)
            break;
         finished = BattleTransitionCommon_UpdateBlackWipe(gBattleTransitionData->data, TRUE, TRUE);
      }
      gBattleTransitionData->tWipeEndY += 8;
      if (gBattleTransitionData->tWipeEndY >= DISPLAY_HEIGHT) {
         gBattleTransitionData->tWipeEndX = DISPLAY_WIDTH;
         task->tState++;
      } else {
         while (gBattleTransitionData->tWipeCurrY < gBattleTransitionData->tWipeEndY)
            gScanlineEffectRegBuffers[0][++gBattleTransitionData->tWipeCurrY] = end | (start << 8);
      }
   }
   
   return FALSE;
}

static bool8 ClockwiseWipe_Bottom(struct Task* task) {
   with_vblank_dma_disabled {
      BattleTransitionCommon_InitBlackWipe(
         gBattleTransitionData->data,
         DISPLAY_WIDTH  / 2,
         DISPLAY_HEIGHT / 2,
         gBattleTransitionData->tWipeEndX,
         DISPLAY_HEIGHT,
         1,
         1
      );
      do {
         gScanlineEffectRegBuffers[0][gBattleTransitionData->tWipeCurrY] = (gBattleTransitionData->tWipeCurrX << 8) | DISPLAY_WIDTH;
      } while (!BattleTransitionCommon_UpdateBlackWipe(gBattleTransitionData->data, TRUE, TRUE));

      gBattleTransitionData->tWipeEndX -= 16;
      if (gBattleTransitionData->tWipeEndX <= 0) {
         gBattleTransitionData->tWipeEndY = DISPLAY_HEIGHT;
         task->tState++;
      }
   }
   return FALSE;
}

static bool8 ClockwiseWipe_Left(struct Task* task) {
   with_vblank_dma_disabled {
      BattleTransitionCommon_InitBlackWipe(
         gBattleTransitionData->data,
         DISPLAY_WIDTH  / 2,
         DISPLAY_HEIGHT / 2,
         0,
         gBattleTransitionData->tWipeEndY,
         1,
         1
      );

      s16 end;
      s16 start;
      s16 temp;
      vu8 finished = FALSE;
      while (1) {
         end   = (gScanlineEffectRegBuffers[0][gBattleTransitionData->tWipeCurrY]) & 0xFF;
         start = gBattleTransitionData->tWipeCurrX;
         if (gBattleTransitionData->tWipeEndY <= DISPLAY_HEIGHT / 2)
            start = DISPLAY_WIDTH / 2, end = gBattleTransitionData->tWipeCurrX;
         temp = end | (start << 8);
         gScanlineEffectRegBuffers[0][gBattleTransitionData->tWipeCurrY] = temp;
         if (finished)
            break;
         finished = BattleTransitionCommon_UpdateBlackWipe(gBattleTransitionData->data, TRUE, TRUE);
      }

      gBattleTransitionData->tWipeEndY -= 8;
      if (gBattleTransitionData->tWipeEndY <= 0) {
         gBattleTransitionData->tWipeEndX = 0;
         task->tState++;
      } else {
         while (gBattleTransitionData->tWipeCurrY > gBattleTransitionData->tWipeEndY)
            gScanlineEffectRegBuffers[0][--gBattleTransitionData->tWipeCurrY] = end | (start << 8);
      }
   }
   return FALSE;
}

static bool8 ClockwiseWipe_TopLeft(struct Task* task) {
   with_vblank_dma_disabled {
      BattleTransitionCommon_InitBlackWipe(gBattleTransitionData->data, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, gBattleTransitionData->tWipeEndX, 0, 1, 1);
      do {
         s16 start, end;
         start = DISPLAY_WIDTH / 2, end = gBattleTransitionData->tWipeCurrX;
         if (gBattleTransitionData->tWipeCurrX >= DISPLAY_WIDTH / 2)
            start = 0, end = DISPLAY_WIDTH;
         gScanlineEffectRegBuffers[0][gBattleTransitionData->tWipeCurrY] = end | (start << 8);
      } while (!BattleTransitionCommon_UpdateBlackWipe(gBattleTransitionData->data, TRUE, TRUE));

      gBattleTransitionData->tWipeEndX += 16;
      if (gBattleTransitionData->tWipeCurrX > DISPLAY_WIDTH / 2)
         task->tState++;
   }
   return FALSE;
}

static bool8 ClockwiseWipe_End(struct Task* task) {
   DmaStop(0);
   BattleTransitionCommon_FadeScreenBlack();
   DestroyTask(FindTaskIdByFunc(BattleTransitionTaskHandler_ClockwiseWipe));
   return FALSE;
}

static void VBlankCB_ClockwiseWipe(void) {
   DmaStop(0);
   BattleTransitionCommon_VBlankCB();
   if (gBattleTransitionData->VBlank_DMA != 0)
      DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 2);
   REG_WININ  = gBattleTransitionData->WININ;
   REG_WINOUT = gBattleTransitionData->WINOUT;
   REG_WIN0V  = gBattleTransitionData->WIN0V;
   REG_WIN0H  = gScanlineEffectRegBuffers[1][0];
   DmaSet(0, gScanlineEffectRegBuffers[1], &REG_WIN0H, B_TRANS_DMA_FLAGS);
}
