#include "battle_transition/common.h"
#include "battle_transition/common_intro.h"
#include "gba/gba.h"
#include "constants/rgb.h" // RGB()
#include "palette.h"
#include "task.h"

// CONFIG

// Before any unique transition effect plays, the game rapidly fades back and 
// forth between full-color and opaque gray. The "increment" options here will 
// influence the fade speed.
//
// Increasing the increment looks really bad and a little seizuriffic.
#define FADE_TO_GRAY_DELAY       0 // vanilla: 0
#define FADE_FROM_GRAY_DELAY     0 // vanilla: 0
#define NUMBER_OF_FADES          1 // vanilla: 3
#define FADE_TO_GRAY_INCREMENT   2 // vanilla: 2
#define FADE_FROM_GRAY_INCREMENT 2 // vanilla: 2

// END CONFIG

#define tFadeToGrayDelay       data[1]
#define tFadeFromGrayDelay     data[2]
#define tNumFades              data[3]
#define tFadeToGrayIncrement   data[4]
#define tFadeFromGrayIncrement data[5]
#define tDelayTimer            data[6]
#define tBlend                 data[7]

static void Task_BattleTransition_Intro(u8 taskId);
//
static bool8 TransitionIntro_FadeToGray(struct Task*);
static bool8 TransitionIntro_FadeFromGray(struct Task*);
//
static const TransitionStateFunc sTransitionIntroFuncs[] = {
   TransitionIntro_FadeToGray,
   TransitionIntro_FadeFromGray
};

extern void BattleTransitionIntro_CreateTask(void) {
   #if NUMBER_OF_FADES <= 0
      //
      // The task logic would loop forever if we started it with a zero 
      // fade count. Other parts of the battle transition code just poll 
      // to see if the task has ceased to exist, so if we never create 
      // it in the first place, then the main transition should play 
      // seamlessly.
      //
      return;
   #endif
   u8 taskId = CreateTask(Task_BattleTransition_Intro, 3);
   gTasks[taskId].tFadeToGrayDelay       = FADE_TO_GRAY_DELAY;
   gTasks[taskId].tFadeFromGrayDelay     = FADE_FROM_GRAY_DELAY;
   gTasks[taskId].tNumFades              = NUMBER_OF_FADES;
   gTasks[taskId].tFadeToGrayIncrement   = FADE_TO_GRAY_INCREMENT;
   gTasks[taskId].tFadeFromGrayIncrement = FADE_FROM_GRAY_INCREMENT;
   //
   gTasks[taskId].tDelayTimer = FADE_TO_GRAY_DELAY;
}

extern bool8 BattleTransitionIntro_IsTaskDone(void) {
    if (FindTaskIdByFunc(Task_BattleTransition_Intro) == TASK_NONE)
        return TRUE;
    else
        return FALSE;
}

static void Task_BattleTransition_Intro(u8 taskId) {
   while (sTransitionIntroFuncs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 TransitionIntro_FadeToGray(struct Task* task) {
    if (task->tDelayTimer == 0 || --task->tDelayTimer == 0)
    {
        task->tDelayTimer = task->tFadeToGrayDelay;
        task->tBlend += task->tFadeToGrayIncrement;
        if (task->tBlend > 16)
            task->tBlend = 16;
        BlendPalettes(PALETTES_ALL, task->tBlend, RGB(11, 11, 11));
    }
    if (task->tBlend >= 16)
    {
        // Fade to gray complete, start fade back
        task->tState++;
        task->tDelayTimer = task->tFadeFromGrayDelay;
    }
    return FALSE;
}

static bool8 TransitionIntro_FadeFromGray(struct Task *task)
{
    if (task->tDelayTimer == 0 || --task->tDelayTimer == 0)
    {
        task->tDelayTimer = task->tFadeFromGrayDelay;
        task->tBlend -= task->tFadeFromGrayIncrement;
        if (task->tBlend < 0)
            task->tBlend = 0;
        BlendPalettes(PALETTES_ALL, task->tBlend, RGB(11, 11, 11));
    }
    if (task->tBlend == 0)
    {
        if (--task->tNumFades == 0)
        {
            // All fades done, end intro
            DestroyTask(FindTaskIdByFunc(Task_BattleTransition_Intro));
        }
        else
        {
            // Fade from gray complete, start new fade
            task->tDelayTimer = task->tFadeToGrayDelay;
            task->tState = 0;
        }
    }
    return FALSE;
}

#undef tFadeToGrayDelay
#undef tFadeFromGrayDelay
#undef tNumFades
#undef tFadeToGrayIncrement
#undef tFadeFromGrayIncrement
#undef tDelayTimer
#undef tBlend