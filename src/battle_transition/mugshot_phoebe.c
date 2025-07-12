#include "battle_transition/common.h"
#include "battle_transition/common_mugshot.h"
#include "battle_transition/mugshot_phoebe.h"
#include "constants/trainers.h"
#include "task.h"

static const u16 sMugshotPal_Phoebe[] = INCBIN_U16("graphics/battle_transitions/phoebe_bg.gbapal");

static const struct BattleTransitionMugshot sMugshotParams = {
   .opponent = {
      .banner_palette = {
         .data = sMugshotPal_Phoebe,
         .size = sizeof(sMugshotPal_Phoebe)
      },
      .trainer_sprite = {
         .affine = {
            .rotation = 0x200,
            .scale    = 0x200
         },
         .coords = {
            .x = 0,
            .y = 0
         },
         .trainer_pic_id = TRAINER_PIC_ELITE_FOUR_PHOEBE
      },
   },
};

void BattleTransitionTaskHandler_MugshotPhoebe(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   BattleTransitionCommon_Mugshot_Exec(task, &sMugshotParams);
}