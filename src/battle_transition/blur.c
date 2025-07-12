#include "battle_transition/common.h"
#include "battle_transition/blur.h"
#include "gba/gba.h"
#include "constants/rgb.h"
#include "gpu_regs.h"
#include "palette.h"
#include "task.h"

static bool8 Blur_Init(struct Task* task);
static bool8 Blur_Main(struct Task* task);
static bool8 Blur_End(struct Task* task);
//
static const TransitionStateFunc sBlur_Funcs[] = {
   Blur_Init,
   Blur_Main,
   Blur_End
};

#define tDelay   data[1]
#define tCounter data[2]

void BattleTransitionTaskHandler_Blur(u8 taskId) {
   while (sBlur_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 Blur_Init(struct Task* task) {
   SetGpuReg(REG_OFFSET_MOSAIC, 0);
   SetGpuRegBits(REG_OFFSET_BG1CNT, BGCNT_MOSAIC);
   SetGpuRegBits(REG_OFFSET_BG2CNT, BGCNT_MOSAIC);
   SetGpuRegBits(REG_OFFSET_BG3CNT, BGCNT_MOSAIC);
   task->tState++;
   return TRUE;
}

static bool8 Blur_Main(struct Task* task) {
   if (task->tDelay != 0) {
      task->tDelay--;
   } else  {
      task->tDelay = 4;
      if (++task->tCounter == 10)
         BeginNormalPaletteFade(PALETTES_ALL, -1, 0, 16, RGB_BLACK);
      SetGpuReg(REG_OFFSET_MOSAIC, (task->tCounter & 15) * 17);
      if (task->tCounter > 14)
         task->tState++;
   }
   return FALSE;
}

static bool8 Blur_End(struct Task* task) {
   if (!gPaletteFade.active) {
      DestroyTask(FindTaskIdByFunc(BattleTransitionTaskHandler_Blur));
   }
   return FALSE;
}

#undef tDelay
#undef tCounter