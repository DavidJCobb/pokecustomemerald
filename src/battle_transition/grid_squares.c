#include "battle_transition/common.h"
#include "battle_transition/assets/field_effect_pal_pokeball.h"
#include "battle_transition/assets/shrinking_box_tileset.h"
#include "battle_transition/grid_squares.h"
#include "gba/gba.h"
#include "gpu_regs.h"
#include "palette.h"
#include "task.h"

static bool8 GridSquares_Init(struct Task* task);
static bool8 GridSquares_Main(struct Task* task);
static bool8 GridSquares_End(struct Task* task);
//
static const TransitionStateFunc sGridSquares_Funcs[] = {
   GridSquares_Init,
   GridSquares_Main,
   GridSquares_End
};

#define tDelay       data[1]
#define tShrinkStage data[2]

void BattleTransitionTaskHandler_GridSquares(u8 taskId) {
   while (sGridSquares_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 GridSquares_Init(struct Task *task) {
   u16 *tilemap, *tileset;

   BattleTransitionCommon_GetBg0TilesDst(&tilemap, &tileset);
   CpuSet(gBattleTransitionAsset_ShrinkingBoxTileset, tileset, 16);
   CpuFill16(0xF0 << 8, tilemap, BG_SCREEN_SIZE);
   LoadPalette(gBattleTransitionAsset_FieldEffectPal_Pokeball, BG_PLTT_ID(15), PLTT_SIZE_4BPP);

   task->tState++;
   return FALSE;
}

static bool8 GridSquares_Main(struct Task *task) {
   u16 *tileset;

   if (task->tDelay == 0) {
      BattleTransitionCommon_GetBg0TilemapDst(&tileset);
      task->tDelay = 3;
      task->tShrinkStage++;
      CpuSet(&gBattleTransitionAsset_ShrinkingBoxTileset[task->tShrinkStage * 8], tileset, 16);
      if (task->tShrinkStage > 13) {
         task->tState++;
         task->tDelay = 16;
      }
   }

   task->tDelay--;
   return FALSE;
}

static bool8 GridSquares_End(struct Task *task) {
   if (--task->tDelay == 0) {
      BattleTransitionCommon_FadeScreenBlack();
      DestroyTask(FindTaskIdByFunc(BattleTransitionTaskHandler_GridSquares));
   }
   return FALSE;
}

#undef tDelay
#undef tShrinkStage