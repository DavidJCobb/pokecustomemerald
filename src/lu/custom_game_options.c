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

u8 ApplyCustomGameScale_u8(u8 v, CustomGameScalePct scale) {
   if (scale != 100) {
      v = ((u32)v * scale) / 100;
      if (v > 255)
         v = 255;
   }
   return v;
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
   
   gCustomGameOptions.daycare_eggs.daycare_can_teach_moves = TRUE;
   gCustomGameOptions.daycare_eggs.daycare_scale_cost      = 100;
   gCustomGameOptions.daycare_eggs.daycare_scale_step_exp  = 100;
   gCustomGameOptions.daycare_eggs.egg_lay_chance          = 100;
   
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
   gCustomGameOptions.scale_exp_gains.trainer_battles = 150;
   
   gCustomGameOptions.scale_player_money_gain_on_victory      = 100;
   gCustomGameOptions.modern_calc_player_money_loss_on_defeat = FALSE;
   
   gCustomGameOptions.events.eon_ticket = FALSE;
   gCustomGameOptions.events.mirage_island.rarity     = 16;
   gCustomGameOptions.events.mirage_island.include_pc = FALSE;
   
   gCustomGameOptions.overworld_poison.interval = 4; // vanilla value; formerly hardcoded
   gCustomGameOptions.overworld_poison.damage   = 1; // vanilla value; formerly hardcoded
   gCustomGameOptions.overworld_poison.faint    = TRUE;
   
   gCustomGameOptions.rematches.min_badge_count = 5;
   gCustomGameOptions.rematches.chance          = 31;
   gCustomGameOptions.rematches.interval        = 255;
   
   gCustomGameOptions.starters.species[0] = 0;
   gCustomGameOptions.starters.species[1] = 0;
   gCustomGameOptions.starters.species[2] = 0;
   gCustomGameOptions.starters.forceGender = CustomGame_PlayerStarterForceGender_Random;
   gCustomGameOptions.starters.level = 5;
}

void ResetCustomGameSavestate(void) {
}

extern void SetCustomGameOptionsPerGeneration(struct CustomGameOptions* dst, u8 generation) {
   if (generation >= 4) {
      gCustomGameOptions.can_run_indoors = TRUE;
      
      gCustomGameOptions.modern_calc_player_money_loss_on_defeat = TRUE;
      
      gCustomGameOptions.overworld_poison.faint = FALSE;
      
      if (generation >= 5) {
         gCustomGameOptions.overworld_poison.interval = 0;
         gCustomGameOptions.overworld_poison.damage   = 0;
         if (generation >= 6) {
            gCustomGameOptions.enable_catch_exp = TRUE;
            if (generation >= 7) {
               gCustomGameOptions.scale_exp_gains.trainer_battles = 100;
            }
         }
      }
   }
}

void CustomGames_HandleNewPlaythroughStarted(void) {
   ResetCustomGameSavestate();
   
   if (gCustomGameOptions.start_with_running_shoes) {
      FlagSet(FLAG_SYS_B_DASH);
   }
}