#include "battle_transition/common.h"
#include "battle_transition/wave.h"
#include "gba/gba.h"
#include "gpu_regs.h"
#include "main.h" // SetVBlankCallback, SetHBlankCallback
#include "scanline_effect.h"
#include "task.h"
#include "trig.h"

static bool8 Wave_Init(struct Task*);
static bool8 Wave_Main(struct Task*);
static bool8 Wave_End(struct Task*);
//
static const TransitionStateFunc sWave_Funcs[] = {
   Wave_Init,
   Wave_Main,
   Wave_End
};

static void VBlankCB_Wave(void);

#define tX        data[1]
#define tSinIndex data[2]

void BattleTransitionTaskHandler_Wave(u8 taskId) {
   while (sWave_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 Wave_Init(struct Task* task) {
   BattleTransitionCommon_InitTransitionData();
   ScanlineEffect_Clear();

   gBattleTransitionData->WININ  = WININ_WIN0_ALL;
   gBattleTransitionData->WINOUT = 0;
   gBattleTransitionData->WIN0H  = DISPLAY_WIDTH;
   gBattleTransitionData->WIN0V  = DISPLAY_HEIGHT;

   for (u8 i = 0; i < DISPLAY_HEIGHT; i++)
      gScanlineEffectRegBuffers[1][i] = DISPLAY_WIDTH + 2;

   SetVBlankCallback(VBlankCB_Wave);

   task->tState++;
   return TRUE;
}

static bool8 Wave_Main(struct Task* task) {
   with_vblank_dma_disabled {
      u16* toStore  = gScanlineEffectRegBuffers[0];
      u8   sinIndex = task->tSinIndex;
      task->tSinIndex += 16;
      task->tX += 8;

      bool8 finished = TRUE;
      for (u8 i = 0; i < DISPLAY_HEIGHT; i++, sinIndex += 4, toStore++) {
         s16 x = task->tX + Sin(sinIndex, 40);
         if (x < 0)
            x = 0;
         if (x > DISPLAY_WIDTH)
            x = DISPLAY_WIDTH;
         *toStore = (x << 8) | (DISPLAY_WIDTH + 1);
         if (x < DISPLAY_WIDTH)
            finished = FALSE;
      }
      if (finished)
         task->tState++;
   }
   return FALSE;
}

static bool8 Wave_End(struct Task* task) {
   DmaStop(0);
   BattleTransitionCommon_FadeScreenBlack();
   DestroyTask(FindTaskIdByFunc(BattleTransitionTaskHandler_Wave));
   return FALSE;
}

static void VBlankCB_Wave(void) {
   DmaStop(0);
   BattleTransitionCommon_VBlankCB();
   if (gBattleTransitionData->VBlank_DMA != 0)
      DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 2);
   REG_WININ  = gBattleTransitionData->WININ;
   REG_WINOUT = gBattleTransitionData->WINOUT;
   REG_WIN0V  = gBattleTransitionData->WIN0V;
   DmaSet(0, gScanlineEffectRegBuffers[1], &REG_WIN0H, B_TRANS_DMA_FLAGS);
}

#undef tSinVal
#undef tAmplitudeVal
#undef tTimer
#undef tFadeStarted