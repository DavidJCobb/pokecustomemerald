#include "battle_transition/common.h"
#include "battle_transition/common_mugshot.h"
#include "battle_transition/mugshot_glacia.h"
#include "constants/trainers.h"
#include "task.h"

static const u16 sMugshotPal_Glacia[] = INCBIN_U16("graphics/battle_transitions/glacia_bg.gbapal");

static const struct BattleTransitionMugshot sMugshotParams = {
   .opponent = {
      .banner_palette = {
         .data = sMugshotPal_Glacia,
         .size = sizeof(sMugshotPal_Glacia)
      },
      .trainer_sprite = {
         .affine = {
            .rotation = 0x1B0,
            .scale    = 0x1B0
         },
         .coords = {
            .x = -4,
            .y = 4
         },
         .trainer_pic_id = TRAINER_PIC_ELITE_FOUR_GLACIA
      },
   },
};

void BattleTransitionTaskHandler_MugshotGlacia(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   BattleTransitionCommon_Mugshot_Exec(task, &sMugshotParams);
}