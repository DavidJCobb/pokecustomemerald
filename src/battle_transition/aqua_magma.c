#include "battle_transition/common.h"
#include "battle_transition/common_pattern_weave.h"
#include "battle_transition/aqua_magma.h"
#include "task.h"

static const u16 sEvilTeam_Palette[] = INCBIN_U16("graphics/battle_transitions/evil_team.gbapal");
static const u32 sTeamAqua_Tileset[] = INCBIN_U32("graphics/battle_transitions/team_aqua.4bpp.lz");
static const u32 sTeamAqua_Tilemap[] = INCBIN_U32("graphics/battle_transitions/team_aqua.bin.lz");
static const u32 sTeamMagma_Tileset[] = INCBIN_U32("graphics/battle_transitions/team_magma.4bpp.lz");
static const u32 sTeamMagma_Tilemap[] = INCBIN_U32("graphics/battle_transitions/team_magma.bin.lz");

static const struct BattleTransitionPatternWeave sPatternWeaveParams_Aqua = {
   .end_delay = 60,
   .palette   = {
      .data = sEvilTeam_Palette,
      .size = sizeof(sEvilTeam_Palette),
   },
   .tilemap = {
      .data = sTeamAqua_Tilemap,
      .size = 0,
      .type = TILEMAP_IS_COMPRESSED,
   },
   .tileset = {
      .data = sTeamAqua_Tileset,
      .size = 0,
      .compressed = TRUE,
   },
};
static const struct BattleTransitionPatternWeave sPatternWeaveParams_Magma = {
   .end_delay = 60,
   .palette   = {
      .data = sEvilTeam_Palette,
      .size = sizeof(sEvilTeam_Palette),
   },
   .tilemap = {
      .data = sTeamMagma_Tilemap,
      .size = 0,
      .type = TILEMAP_IS_COMPRESSED,
   },
   .tileset = {
      .data = sTeamMagma_Tileset,
      .size = 0,
      .compressed = TRUE,
   },
};

void BattleTransitionTaskHandler_Aqua(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   BattleTransitionCommon_PatternWeave_Exec(task, &sPatternWeaveParams_Aqua);
}
void BattleTransitionTaskHandler_Magma(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   BattleTransitionCommon_PatternWeave_Exec(task, &sPatternWeaveParams_Magma);
}