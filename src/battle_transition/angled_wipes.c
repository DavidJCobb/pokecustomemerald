#include "battle_transition/common.h"
#include "battle_transition/common_black_wipe.h"
#include "battle_transition/angled_wipes.h"
#include "gba/gba.h"
#include "gpu_regs.h"
#include "main.h" // SetHBlankCallback, SetVBlankCallback
#include "scanline_effect.h" // gScanlineEffectRegBuffers
#include "task.h"

// CONFIGURATION

#define NUM_ANGLED_WIPES 7

static const s16 sAngledWipes_MoveData[NUM_ANGLED_WIPES][5] = {
   // startX       startY          endX            endY            yDirection
   {56,            0,              0,              DISPLAY_HEIGHT, 0},
   {104,           DISPLAY_HEIGHT, DISPLAY_WIDTH,  88,             1},
   {DISPLAY_WIDTH, 72,             56,             0,              1},
   {0,             32,             144,            DISPLAY_HEIGHT, 0},
   {144,           DISPLAY_HEIGHT, 184,            0,              1},
   {56,            0,              168,            DISPLAY_HEIGHT, 0},
   {168,           DISPLAY_HEIGHT, 48,             0,              1},
};
static const s16 sAngledWipes_EndDelays[NUM_ANGLED_WIPES] = {8, 4, 2, 1, 1, 1, 0};

// END CONFIGURATION

static bool8 AngledWipes_Init(struct Task*);
static bool8 AngledWipes_SetWipeData(struct Task*);
static bool8 AngledWipes_DoWipe(struct Task*);
static bool8 AngledWipes_TryEnd(struct Task*);
static bool8 AngledWipes_StartNext(struct Task*);
//
static const TransitionStateFunc sAngledWipes_Funcs[] = {
   AngledWipes_Init,
   AngledWipes_SetWipeData,
   AngledWipes_DoWipe,
   AngledWipes_TryEnd,
   AngledWipes_StartNext,
};

static void VBlankCB_AngledWipes(void);

#define tWipeId data[1]
#define tDir    data[2]
#define tDelay  data[3]

void BattleTransitionTaskHandler_AngledWipes(u8 taskId) {
   while (sAngledWipes_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 AngledWipes_Init(struct Task* task) {
   BattleTransitionCommon_InitTransitionData();
   ScanlineEffect_Clear();

   gBattleTransitionData->WININ  = WININ_WIN0_ALL;
   gBattleTransitionData->WINOUT = 0;
   gBattleTransitionData->WIN0V  = DISPLAY_HEIGHT;

   for (u16 i = 0; i < DISPLAY_HEIGHT; i++)
      gScanlineEffectRegBuffers[0][i] = DISPLAY_WIDTH;

   CpuSet(gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT);
   SetVBlankCallback(VBlankCB_AngledWipes);

   task->tState++;
   return TRUE;
}

static bool8 AngledWipes_SetWipeData(struct Task* task) {
   BattleTransitionCommon_InitBlackWipe(
      gBattleTransitionData->data,
      sAngledWipes_MoveData[task->tWipeId][0],
      sAngledWipes_MoveData[task->tWipeId][1],
      sAngledWipes_MoveData[task->tWipeId][2],
      sAngledWipes_MoveData[task->tWipeId][3],
      1,
      1
   );
   task->tDir = sAngledWipes_MoveData[task->tWipeId][4];
   task->tState++;
   return TRUE;
}

static bool8 AngledWipes_DoWipe(struct Task* task) {
   with_vblank_dma_disabled {
      bool8 finished;
      for (s16 i = 0, finished = FALSE; i < 16; i++) {
         s16 r3 = gScanlineEffectRegBuffers[0][gBattleTransitionData->tWipeCurrY] >> 8;
         s16 r4 = gScanlineEffectRegBuffers[0][gBattleTransitionData->tWipeCurrY] & 0xFF;
         if (task->tDir == 0) {
            // Moving down
            if (r3 < gBattleTransitionData->tWipeCurrX)
                r3 = gBattleTransitionData->tWipeCurrX;
            if (r3 > r4)
                r3 = r4;
         } else {
            // Moving up
            if (r4 > gBattleTransitionData->tWipeCurrX)
                r4 = gBattleTransitionData->tWipeCurrX;
            if (r4 <= r3)
                r4 = r3;
         }
         gScanlineEffectRegBuffers[0][gBattleTransitionData->tWipeCurrY] = (r4) | (r3 << 8);
         if (finished) {
            task->tState++;
            break;
         }
         finished = BattleTransitionCommon_UpdateBlackWipe(gBattleTransitionData->data, TRUE, TRUE);
      }
   }
   return FALSE;
}

static bool8 AngledWipes_TryEnd(struct Task* task) {
   if (++task->tWipeId < NUM_ANGLED_WIPES) {
      //
      // Continue with next wipe.
      //
      task->tState++;
      task->tDelay = sAngledWipes_EndDelays[task->tWipeId - 1];
      return TRUE;
   } else {
      DmaStop(0);
      BattleTransitionCommon_FadeScreenBlack();
      DestroyTask(FindTaskIdByFunc(BattleTransitionTaskHandler_AngledWipes));
      return FALSE;
   }
}

static bool8 AngledWipes_StartNext(struct Task* task) {
   if (--task->tDelay == 0) {
      // Return to AngledWipes_SetWipeData
      task->tState = 1;
      return TRUE;
   }
   return FALSE;
}

#undef tWipeId
#undef tDir
#undef tDelay

static void VBlankCB_AngledWipes(void) {
   DmaStop(0);
   BattleTransitionCommon_VBlankCB();
   if (gBattleTransitionData->VBlank_DMA)
      DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 2);
   REG_WININ  = gBattleTransitionData->WININ;
   REG_WINOUT = gBattleTransitionData->WINOUT;
   REG_WIN0V  = gBattleTransitionData->WIN0V;
   REG_WIN0H  = gScanlineEffectRegBuffers[1][0];
   DmaSet(0, gScanlineEffectRegBuffers[1], &REG_WIN0H, B_TRANS_DMA_FLAGS);
}
