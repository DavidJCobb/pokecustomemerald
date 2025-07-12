#include "battle_transition/common.h"
#include "battle_transition/shuffle.h"
#include <string.h> // memset
#include "gba/gba.h"
#include "gpu_regs.h"
#include "constants/rgb.h"
#include "main.h" // SetHBlankCallback, SetVBlankCallback
#include "palette.h"
#include "scanline_effect.h" // gScanlineEffectRegBuffers
#include "task.h"
#include "trig.h"

static void VBlankCB_Shuffle(void);
static void HBlankCB_Shuffle(void);

static bool8 Shuffle_Init(struct Task* task);
static bool8 Shuffle_End(struct Task* task);
//
static const TransitionStateFunc sShuffle_Funcs[] = {
   Shuffle_Init,
   Shuffle_End,
};

#define tSinVal    data[1]
#define tAmplitude data[2]

void BattleTransitionTaskHandler_Shuffle(u8 taskId) {
   while (sShuffle_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 Shuffle_Init(struct Task* task) {
   BattleTransitionCommon_InitTransitionData();
   ScanlineEffect_Clear();

   BeginNormalPaletteFade(PALETTES_ALL, 4, 0, 16, RGB_BLACK);
   memset(gScanlineEffectRegBuffers[1], gBattleTransitionData->cameraY, DISPLAY_HEIGHT * 2);

   SetVBlankCallback(VBlankCB_Shuffle);
   SetHBlankCallback(HBlankCB_Shuffle);

   EnableInterrupts(INTR_FLAG_VBLANK | INTR_FLAG_HBLANK);

   task->tState++;
   return FALSE;
}

static bool8 Shuffle_End(struct Task* task) {
   with_vblank_dma_disabled {
      u16 sinVal    = task->tSinVal;
      u16 amplitude = task->tAmplitude >> 8;
      task->tSinVal    += 4224;
      task->tAmplitude += 384;

      for (u8 i = 0; i < DISPLAY_HEIGHT; i++, sinVal += 4224) {
         u16 sinIndex = sinVal / 256;
         gScanlineEffectRegBuffers[0][i] = gBattleTransitionData->cameraY + Sin(sinIndex, amplitude);
      }

      if (!gPaletteFade.active)
         DestroyTask(FindTaskIdByFunc(BattleTransitionTaskHandler_Shuffle));
   }
   return FALSE;
}

static void VBlankCB_Shuffle(void) {
   BattleTransitionCommon_VBlankCB();
   if (gBattleTransitionData->VBlank_DMA)
      DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 2);
}

static void HBlankCB_Shuffle(void) {
   u16 var = gScanlineEffectRegBuffers[1][REG_VCOUNT];
   REG_BG1VOFS = var;
   REG_BG2VOFS = var;
   REG_BG3VOFS = var;
}

#undef tSinVal
#undef tAmplitude