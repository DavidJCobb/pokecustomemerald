#include "battle_transition/common.h"
#include "battle_transition/slice.h"
#include "gba/gba.h"
#include "gpu_regs.h"
#include "main.h" // SetHBlankCallback, SetVBlankCallback
#include "scanline_effect.h" // gScanlineEffectRegBuffers
#include "task.h"

#define SLICE_TRANSITION_INITIAL_SPEED 4  // vanilla: 1
#define SLICE_TRANSITION_INITIAL_ACCEL 64 // vanilla: 1

static void VBlankCB_Slice(void);
static void HBlankCB_Slice(void);

static bool8 Slice_Init(struct Task* task);
static bool8 Slice_Main(struct Task* task);
static bool8 Slice_End(struct Task* task);
//
static const TransitionStateFunc sSlice_Funcs[] = {
   Slice_Init,
   Slice_Main,
   Slice_End
};

void BattleTransitionTaskHandler_Slice(u8 taskId) {
   while (sSlice_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

#define tEffectX data[1]
#define tSpeed   data[2]
#define tAccel   data[3]

static bool8 Slice_Init(struct Task* task) {
   u16 i;

   BattleTransitionCommon_InitTransitionData();
   ScanlineEffect_Clear();

   task->tSpeed = SLICE_TRANSITION_INITIAL_SPEED << 8;
   task->tAccel = SLICE_TRANSITION_INITIAL_ACCEL;
   gBattleTransitionData->VBlank_DMA = FALSE;

   for (i = 0; i < DISPLAY_HEIGHT; i++) {
      gScanlineEffectRegBuffers[1][i] = gBattleTransitionData->cameraX;
      gScanlineEffectRegBuffers[1][DISPLAY_HEIGHT + i] = DISPLAY_WIDTH;
   }

   EnableInterrupts(INTR_FLAG_HBLANK);
   SetGpuRegBits(REG_OFFSET_DISPSTAT, DISPSTAT_HBLANK_INTR);

   SetVBlankCallback(VBlankCB_Slice);
   SetHBlankCallback(HBlankCB_Slice);

   task->tState++;
   return TRUE;
}

static bool8 Slice_Main(struct Task* task) {
   with_vblank_dma_disabled {
      task->tEffectX += (task->tSpeed >> 8);
      if (task->tEffectX > DISPLAY_WIDTH)
         task->tEffectX = DISPLAY_WIDTH;
      if (task->tSpeed <= 0xFFF)
         task->tSpeed += task->tAccel;
      if (task->tAccel < 128)
         task->tAccel <<= 1; // multiplying by two

      for (u16 i = 0; i < DISPLAY_HEIGHT; i++) {
         u16* storeLoc1 = &gScanlineEffectRegBuffers[0][i];
         u16* storeLoc2 = &gScanlineEffectRegBuffers[0][i + DISPLAY_HEIGHT];

         // Alternate rows
         if (i % 2) {
            *storeLoc1 = gBattleTransitionData->cameraX + task->tEffectX;
            *storeLoc2 = DISPLAY_WIDTH - task->tEffectX;
         } else {
            *storeLoc1 = gBattleTransitionData->cameraX - task->tEffectX;
            *storeLoc2 = (task->tEffectX << 8) | (DISPLAY_WIDTH + 1);
         }
      }

      if (task->tEffectX >= DISPLAY_WIDTH)
         task->tState++;
   }
   return FALSE;
}

static bool8 Slice_End(struct Task* task) {
   DmaStop(0);
   BattleTransitionCommon_FadeScreenBlack();
   DestroyTask(FindTaskIdByFunc(BattleTransitionTaskHandler_Slice));
   return FALSE;
}

static void VBlankCB_Slice(void) {
   DmaStop(0);
   BattleTransitionCommon_VBlankCB();
   REG_WININ  = WININ_WIN0_ALL;
   REG_WINOUT = 0;
   REG_WIN0V  = DISPLAY_HEIGHT;
   if (gBattleTransitionData->VBlank_DMA)
      DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 4);
   DmaSet(0, &gScanlineEffectRegBuffers[1][DISPLAY_HEIGHT], &REG_WIN0H, B_TRANS_DMA_FLAGS);
}

static void HBlankCB_Slice(void) {
   if (REG_VCOUNT < DISPLAY_HEIGHT) {
      u16 var = gScanlineEffectRegBuffers[1][REG_VCOUNT];
      REG_BG1HOFS = var;
      REG_BG2HOFS = var;
      REG_BG3HOFS = var;
   }
}

#undef tEffectX
#undef tSpeed
#undef tAccel