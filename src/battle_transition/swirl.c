#include "battle_transition/common.h"
#include "battle_transition/swirl.h"
#include "gba/gba.h"
#include "constants/rgb.h"
#include "gpu_regs.h"
#include "main.h" // SetHBlankCallback, SetVBlankCallback
#include "scanline_effect.h"
#include "palette.h"
#include "task.h"
#include "trig.h"

#define SWIRL_TRANSITION_DEGREE_INCREMENT    4 // vanilla: 4
#define SWIRL_TRANSITION_AMPLITUDE_INCREMENT 8 // vanilla: 8

static void VBlankCB_Swirl(void);
static void HBlankCB_Swirl(void);

static bool8 Swirl_Init(struct Task* task);
static bool8 Swirl_Exec(struct Task* task);
//
static const TransitionStateFunc sSwirl_Funcs[] = {
   Swirl_Init,
   Swirl_Exec,
};

#define tSinIndex  data[1]
#define tAmplitude data[2]

void BattleTransitionTaskHandler_Swirl(u8 taskId) {
   while (sSwirl_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 Swirl_Init(struct Task* task) {
   BattleTransitionCommon_InitTransitionData();
   ScanlineEffect_Clear();
   BeginNormalPaletteFade(PALETTES_ALL, 4, 0, 16, RGB_BLACK);
   BattleTransitionCommon_SetSinWave(gScanlineEffectRegBuffers[1], gBattleTransitionData->cameraX, 0, 2, 0, DISPLAY_HEIGHT);

   SetVBlankCallback(VBlankCB_Swirl);
   SetHBlankCallback(HBlankCB_Swirl);

   EnableInterrupts(INTR_FLAG_VBLANK | INTR_FLAG_HBLANK);

   task->tState++;
   return FALSE;
}

static bool8 Swirl_Exec(struct Task* task) {
   with_vblank_dma_disabled {
      task->tSinIndex  += SWIRL_TRANSITION_DEGREE_INCREMENT;
      task->tAmplitude += SWIRL_TRANSITION_AMPLITUDE_INCREMENT;

      BattleTransitionCommon_SetSinWave(
         gScanlineEffectRegBuffers[0],
         gBattleTransitionData->cameraX,
         task->tSinIndex,
         2,
         task->tAmplitude,
         DISPLAY_HEIGHT
      );

      if (!gPaletteFade.active) {
         u8 taskId = FindTaskIdByFunc(BattleTransitionTaskHandler_Swirl);
         DestroyTask(taskId);
      }
   }
   return FALSE;
}

static void VBlankCB_Swirl(void) {
   BattleTransitionCommon_VBlankCB();
   if (gBattleTransitionData->VBlank_DMA)
      DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 2);
}

static void HBlankCB_Swirl(void) {
   u16 var = gScanlineEffectRegBuffers[1][REG_VCOUNT];
   REG_BG1HOFS = var;
   REG_BG2HOFS = var;
   REG_BG3HOFS = var;
}

#undef tSinIndex
#undef tAmplitude