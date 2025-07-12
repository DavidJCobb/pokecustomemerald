#include "battle_transition/common.h"
#include "battle_transition/ripple.h"
#include "gba/gba.h"
#include "gpu_regs.h"
#include "constants/rgb.h"
#include "main.h" // SetVBlankCallback, SetHBlankCallback
#include "palette.h"
#include "scanline_effect.h"
#include "task.h"
#include "trig.h"

static bool8 Ripple_Init(struct Task*);
static bool8 Ripple_Main(struct Task*);
//
static const TransitionStateFunc sRipple_Funcs[] = {
   Ripple_Init,
   Ripple_Main
};

static void VBlankCB_Ripple(void);
static void HBlankCB_Ripple(void);

#define tSinVal       data[1]
#define tAmplitudeVal data[2]
#define tTimer        data[3]
#define tFadeStarted  data[4]

void BattleTransitionTaskHandler_Ripple(u8 taskId) {
   while (sRipple_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 Ripple_Init(struct Task* task) {
   BattleTransitionCommon_InitTransitionData();
   ScanlineEffect_Clear();

   for (u8 i = 0; i < DISPLAY_HEIGHT; i++)
      gScanlineEffectRegBuffers[1][i] = gBattleTransitionData->cameraY;

   SetVBlankCallback(VBlankCB_Ripple);
   SetHBlankCallback(HBlankCB_Ripple);

   EnableInterrupts(INTR_FLAG_HBLANK);

   task->tState++;
   return TRUE;
}

static bool8 Ripple_Main(struct Task* task) {
   with_vblank_dma_disabled {
      s16 amplitude = task->tAmplitudeVal >> 8;
      u16 sinVal    = task->tSinVal;
      u16 speed     = 0x180;
      task->tSinVal += 0x400;
      if (task->tAmplitudeVal <= 0x1FFF)
         task->tAmplitudeVal += 0x180;

      for (u8 i = 0; i < DISPLAY_HEIGHT; i++, sinVal += speed) {
         s16 sinIndex = sinVal >> 8;
         gScanlineEffectRegBuffers[0][i] = gBattleTransitionData->cameraY + Sin(sinIndex & 0xffff, amplitude);
      }

      if (++task->tTimer == 81) {
         task->tFadeStarted++;
         BeginNormalPaletteFade(PALETTES_ALL, -2, 0, 16, RGB_BLACK);
      }

      if (task->tFadeStarted && !gPaletteFade.active)
         DestroyTask(FindTaskIdByFunc(BattleTransitionTaskHandler_Ripple));
   }
   return FALSE;
}

static void VBlankCB_Ripple(void) {
   BattleTransitionCommon_VBlankCB();
   if (gBattleTransitionData->VBlank_DMA)
      DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 2);
}

static void HBlankCB_Ripple(void) {
   u16 var = gScanlineEffectRegBuffers[1][REG_VCOUNT];
   REG_BG1VOFS = var;
   REG_BG2VOFS = var;
   REG_BG3VOFS = var;
}

#undef tSinVal
#undef tAmplitudeVal
#undef tTimer
#undef tFadeStarted