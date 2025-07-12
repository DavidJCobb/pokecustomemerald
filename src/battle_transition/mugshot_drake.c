#include "battle_transition/common.h"
#include "battle_transition/common_mugshot.h"
#include "battle_transition/mugshot_drake.h"
#include "constants/trainers.h"
#include "task.h"

static const u16 sMugshotPal_Drake[] = INCBIN_U16("graphics/battle_transitions/drake_bg.gbapal");

static const struct BattleTransitionMugshot sMugshotParams = {
   .opponent = {
      .banner_palette = {
         .data = sMugshotPal_Drake,
         .size = sizeof(sMugshotPal_Drake)
      },
      .trainer_sprite = {
         .affine = {
            .rotation = 0x1A0,
            .scale    = 0x1A0
         },
         .coords = {
            .x = 0,
            .y = 5
         },
         .trainer_pic_id = TRAINER_PIC_ELITE_FOUR_DRAKE
      },
   },
};

void BattleTransitionTaskHandler_MugshotDrake(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   BattleTransitionCommon_Mugshot_Exec(task, &sMugshotParams);
}