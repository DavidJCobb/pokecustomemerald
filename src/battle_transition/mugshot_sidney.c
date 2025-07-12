#include "battle_transition/common.h"
#include "battle_transition/common_mugshot.h"
#include "battle_transition/mugshot_sidney.h"
#include "constants/trainers.h"
#include "task.h"

static const u16 sMugshotPal_Sidney[] = INCBIN_U16("graphics/battle_transitions/sidney_bg.gbapal");

static const struct BattleTransitionMugshot sMugshotParams = {
   .opponent = {
      .banner_palette = {
         .data = sMugshotPal_Sidney,
         .size = sizeof(sMugshotPal_Sidney)
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
         .trainer_pic_id = TRAINER_PIC_ELITE_FOUR_SIDNEY
      },
   },
};

void BattleTransitionTaskHandler_MugshotSidney(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   BattleTransitionCommon_Mugshot_Exec(task, &sMugshotParams);
}