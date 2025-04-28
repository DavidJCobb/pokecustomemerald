#include "lu/battle_ambient_weather/shared.h"
#include "gba/defines.h"
#include "gba/types.h"
#include "util.h" // BlendPalette
#include "task.h"

// this initializer doesn't actually work lmao.
// the variable is zeroed anyway. brilliant.
EWRAM_DATA u8 gAmbientWeatherTaskId = TASK_NONE;

extern u16 GetActiveAmbientWeather(void) {
   if (gAmbientWeatherTaskId == TASK_NONE)
      return 0;
   return gTasks[gAmbientWeatherTaskId].tWeatherType;
}
extern void TaskHelper_QueueBlendColors(u8 taskId, u32 palettes, u8 delay, u8 blendFrom, u8 blendTo, u16 blendColor) {
   struct Task* task = &gTasks[taskId];
   task->tTargetPalettesA = palettes & 0xFFFF;
   task->tTargetPalettesB = palettes >> 16;
   task->tDelay           = delay << 8;
   task->tBlendRange      = (blendFrom << 8) | blendTo;
   task->tBlendColor      = blendColor;
}
extern bool8 TaskHelper_BlendColors(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   u8 delay = task->tDelay >> 8;
   u8 timer = task->tDelay & 0xFF;
   if (timer == delay) {
      task->tDelay &= 0xFF00;
      
      u8  blend  = task->tBlendRange >> 8;
      u8  target = task->tBlendRange & 0xFF;
      
      u32 palettes = task->tTargetPalettesA | (task->tTargetPalettesB << 16);
      u16 offset   = 0;
      while (palettes != 0) {
         if (palettes & 1) {
            BlendPalette(offset, 16, blend, task->tBlendColor);
         }
         offset += 16;
         palettes >>= 1;
      }
      if (blend < target) {
         ++blend;
      } else if (blend > target) {
         --blend;
      } else {
         return TRUE;
      }
      task->tBlendRange = (blend << 8) | target;
   } else {
      ++task->tDelay;
   }
   return FALSE;
}