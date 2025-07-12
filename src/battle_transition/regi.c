#include "battle_transition/common.h"
#include "battle_transition/common_pattern_weave.h"
#include "battle_transition/regi.h"
#include "task.h"

static const u32 sRegis_Tileset[] = INCBIN_U32("graphics/battle_transitions/regis.4bpp");
static const u16 sRegice_Palette[] = INCBIN_U16("graphics/battle_transitions/regice.gbapal");
static const u16 sRegirock_Palette[] = INCBIN_U16("graphics/battle_transitions/regirock.gbapal");
static const u16 sRegisteel_Palette[] = INCBIN_U16("graphics/battle_transitions/registeel.gbapal");
static const u32 sRegice_Tilemap[] = INCBIN_U32("graphics/battle_transitions/regice.bin");
static const u32 sRegirock_Tilemap[] = INCBIN_U32("graphics/battle_transitions/regirock.bin");
static const u32 sRegisteel_Tilemap[] = INCBIN_U32("graphics/battle_transitions/registeel.bin");

static const struct BattleTransitionPatternWeave sPatternWeaveParams_Regice = {
   .end_delay = 0,
   .palette   = {
      .data = sRegice_Palette,
      .size = sizeof(sRegice_Palette),
   },
   .tilemap = {
      .data = sRegice_Tilemap,
      .size = 0x500,
      .type = TILEMAP_IS_UNCOMPRESSED,
   },
   .tileset = {
      .data = sRegis_Tileset,
      .size = 0x2000,
      .compressed = FALSE,
   },
};
static const struct BattleTransitionPatternWeave sPatternWeaveParams_Regirock = {
   .end_delay = 0,
   .palette   = {
      .data = sRegirock_Palette,
      .size = sizeof(sRegirock_Palette),
   },
   .tilemap = {
      .data = sRegirock_Tilemap,
      .size = 0x500,
      .type = TILEMAP_IS_UNCOMPRESSED,
   },
   .tileset = {
      .data = sRegis_Tileset,
      .size = 0x2000,
      .compressed = FALSE,
   },
};
static const struct BattleTransitionPatternWeave sPatternWeaveParams_Registeel = {
   .end_delay = 0,
   .palette   = {
      .data = sRegisteel_Palette,
      .size = sizeof(sRegisteel_Palette),
   },
   .tilemap = {
      .data = sRegisteel_Tilemap,
      .size = 0x500,
      .type = TILEMAP_IS_UNCOMPRESSED,
   },
   .tileset = {
      .data = sRegis_Tileset,
      .size = 0x2000,
      .compressed = FALSE,
   },
};

void BattleTransitionTaskHandler_Regice(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   BattleTransitionCommon_PatternWeave_Exec(task, &sPatternWeaveParams_Regice);
}
void BattleTransitionTaskHandler_Regirock(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   BattleTransitionCommon_PatternWeave_Exec(task, &sPatternWeaveParams_Regirock);
}
void BattleTransitionTaskHandler_Registeel(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   BattleTransitionCommon_PatternWeave_Exec(task, &sPatternWeaveParams_Registeel);
}