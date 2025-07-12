#include "battle_transition/common.h"
#include "battle_transition/common_pattern_weave.h"
#include "battle_transition/big_pokeball.h"
#include "task.h"

static const u16 sFieldEffectPal_Pokeball[] = INCBIN_U16("graphics/field_effects/palettes/pokeball.gbapal");

static const u16 sBigPokeball_Tilemap[] = INCBIN_U16("graphics/battle_transitions/big_pokeball_map.bin");
static const u32 sBigPokeball_Tileset[] = INCBIN_U32("graphics/battle_transitions/big_pokeball.4bpp");

static const struct BattleTransitionPatternWeave sPatternWeaveParams = {
   .end_delay = 0,
   .palette   = {
      .data = sFieldEffectPal_Pokeball,
      .size = sizeof(sFieldEffectPal_Pokeball),
   },
   .tilemap = {
      .data = sBigPokeball_Tilemap,
      .rect = {
         .x = 30,
         .y = 20,
      },
      .type = TILEMAP_IS_RECT,
   },
   .tileset = {
      .data = sBigPokeball_Tileset,
      .size = sizeof(sBigPokeball_Tileset),
      .compressed = FALSE,
   },
};

void BattleTransitionTaskHandler_BigPokeball(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   BattleTransitionCommon_PatternWeave_Exec(task, &sPatternWeaveParams);
}