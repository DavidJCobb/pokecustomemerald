#include "battle_transition/common.h"
#include "battle_transition/common_mugshot.h"
#include "battle_transition/mugshot_champion.h"
#include "constants/trainers.h"
#include "task.h"

static const u16 sMugshotPal_Champion[] = INCBIN_U16("graphics/battle_transitions/wallace_bg.gbapal");

static const struct BattleTransitionMugshot sMugshotParams = {
   .opponent = {
      .banner_palette = {
         .data = sMugshotPal_Champion,
         .size = sizeof(sMugshotPal_Champion)
      },
      .trainer_sprite = {
         .affine = {
            .rotation = 0x188,
            .scale    = 0x188
         },
         .coords = {
            .x = -8,
            .y = 7
         },
         .trainer_pic_id = TRAINER_PIC_CHAMPION_WALLACE
      },
   },
};

void BattleTransitionTaskHandler_MugshotChampion(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   BattleTransitionCommon_Mugshot_Exec(task, &sMugshotParams);
}