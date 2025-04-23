#include "global.h"
#include "lu/custom_game_options.h"
#include "event_data.h" // FlagSet

EWRAM_DATA struct CustomGameOptions   gCustomGameOptions   = {};
EWRAM_DATA struct CustomGameSavestate gCustomGameSavestate = {};

static void InitializeScaleAndClamp(struct CustomGameScaleAndClamp* v) {
   v->scale = 100;
   v->min   = 0;
   v->max   = 0xFFFF;
}

u16 ApplyCustomGameScale_u16(u16 v, CustomGameScalePct scale) {
   if (scale != 100) {
      v = ((u32)v * scale) / 100;
   }
   return v;
}
u32 ApplyCustomGameScale_u32(u32 v, CustomGameScalePct scale) {
   if (scale != 100) {
      v = (v * scale) / 100;
   }
   return v;
}
s32 ApplyCustomGameScale_s32(s32 v, CustomGameScalePct scale) {
   if (scale != 100) {
      v = (v * scale) / 100;
   }
   return v;
}

u16 ApplyCustomGameScaleAndClamp_u16(u16 v, const struct CustomGameScaleAndClamp* params) {
   if (params->scale != 100) {
      u32 scaled = v;
      scaled *= params->scale;
      scaled /= 100;
   }
   if (v < params->min)
      v = params->min;
   else if (v > params->max)
      v = params->max;
   return v;
}

void ResetCustomGameOptions(void) {
   gCustomGameOptions.start_with_running_shoes = FALSE;
   gCustomGameOptions.can_run_indoors          = FALSE;
   gCustomGameOptions.can_bike_indoors         = FALSE;
   
   gCustomGameOptions.scale_wild_encounter_rate = 100;
   
   gCustomGameOptions.enable_catch_exp = FALSE;
   gCustomGameOptions.catch_rate_scale = 100;
   gCustomGameOptions.catch_rate_increase_base = 0;
   
   gCustomGameOptions.battle_item_usage = CustomGame_ItemUseInBattles_Enabled;
   
   gCustomGameOptions.scale_battle_damage.by_player = 100;
   gCustomGameOptions.scale_battle_damage.by_enemy  = 100;
   gCustomGameOptions.scale_battle_damage.by_ally   = 100;
   
   gCustomGameOptions.scale_battle_accuracy.player = 100;
   gCustomGameOptions.scale_battle_accuracy.enemy  = 100;
   gCustomGameOptions.scale_battle_accuracy.ally   = 100;
   
   gCustomGameOptions.scale_exp_gains.normal = 100;
   gCustomGameOptions.scale_exp_gains.traded = 150;
   
   gCustomGameOptions.scale_player_money_gain_on_victory      = 100;
   gCustomGameOptions.modern_calc_player_money_loss_on_defeat = FALSE;
   
   gCustomGameOptions.overworld_poison.interval = 4; // vanilla value; formerly hardcoded
   gCustomGameOptions.overworld_poison.damage   = 1; // vanilla value; formerly hardcoded
   gCustomGameOptions.overworld_poison.faint    = TRUE;
   
   gCustomGameOptions.starters.species[0] = 0;
   gCustomGameOptions.starters.species[1] = 0;
   gCustomGameOptions.starters.species[2] = 0;
   gCustomGameOptions.starters.forceGender = CustomGame_PlayerStarterForceGender_Random;
   gCustomGameOptions.starters.level = 5;
}

void ResetCustomGameSavestate(void) {
}

void CustomGames_HandleNewPlaythroughStarted(void) {
   ResetCustomGameSavestate();
   
   if (gCustomGameOptions.start_with_running_shoes) {
      FlagSet(FLAG_SYS_B_DASH);
   }
}