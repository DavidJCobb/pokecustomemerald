#include "lu/battle_ambient_weather/shared.h"
#include "gba/defines.h"
#include "gba/types.h"
#include "util.h" // BlendPalette
#include "sound.h" // PlaySE12WithPanning
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
      
      u32 palettes = task->tTargetPalettesA | ((u32)task->tTargetPalettesB << 16);
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

#define tSongId         data[0]
#define tPanning        data[1]
#define tLengthInFrames data[2]
#define tLoopCount      data[3]
#define tTimer          data[4]
#define tOwnerTaskId    data[5]

static void Task_SELoop(u8);

extern void SpawnSELoopTask(
   u8  parent_task,
   u16 song_id,
   s16 panning,
   u16 length_in_frames,
   u16 loop_count // 0xFFFF = infinite
) {
   u8 taskId = CreateTask(Task_SELoop, 1);
   gTasks[taskId].tSongId         = song_id;
   gTasks[taskId].tPanning        = panning;
   gTasks[taskId].tLengthInFrames = length_in_frames;
   gTasks[taskId].tLoopCount      = loop_count;
   gTasks[taskId].tTimer          = length_in_frames;
   gTasks[taskId].tOwnerTaskId    = parent_task;
   gTasks[taskId].func(taskId);
   gTasks[parent_task].tLoopSEChildTask = taskId;
}
extern void DestroySELoopTask(
   u8 parent_task
) {
   u8 taskId = gTasks[parent_task].tLoopSEChildTask;
   if (taskId != TASK_NONE) {
      DestroyTask(taskId);
      gTasks[parent_task].tLoopSEChildTask = TASK_NONE;
   }
}

static void Task_SELoop(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   
   if (task->tTimer++ < task->tLengthInFrames)
      return;
   task->tTimer = 0;
   if (task->tLoopCount != 0xFFFF)
      --task->tLoopCount;
   PlaySE12WithPanning(task->tSongId, task->tPanning);
   if (task->tLoopCount == 0) {
      gTasks[task->tOwnerTaskId].tLoopSEChildTask = TASK_NONE;
      DestroyTask(taskId);
   }
}