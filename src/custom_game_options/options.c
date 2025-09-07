#include "custom_game_options/options.h"

EWRAM_DATA struct CustomGameOptionsSet gCustomGameOptions   = {};
EWRAM_DATA struct CustomGameSavestate  gCustomGameSavestate = {};

static void InitializeScaleAndClamp(struct CustomGameScaleAndClamp* v) {
   v->scale = 100;
   v->min   = 0;
   v->max   = 0xFFFF;
}

void ResetCustomGameOptions(void) {
   gCustomGameOptions.start_with_running_shoes = FALSE;
   gCustomGameOptions.can_run_indoors          = FALSE;
   gCustomGameOptions.can_bike_indoors         = FALSE;
   
   gCustomGameOptions.daycare_eggs.can_teach_moves = TRUE;
   gCustomGameOptions.daycare_eggs.scale_cost      = 100;
   gCustomGameOptions.daycare_eggs.scale_step_exp  = 100;
   gCustomGameOptions.daycare_eggs.egg_lay_chance  = 100;
   
   gCustomGameOptions.battle.catching.enable_catch_exp = FALSE;
   gCustomGameOptions.battle.catching.catch_rate_scale = 100;
   gCustomGameOptions.battle.catching.catch_rate_increase_base = 0;
   
   gCustomGameOptions.battle.item_usage = CGO_BATTLEITEMUSAGE_ENABLED;
   
   gCustomGameOptions.battle.money.scale_gain_on_victory        = 100;
   gCustomGameOptions.battle.money.modern_calc_loss_on_defeat   = FALSE;
   gCustomGameOptions.battle.money.lose_money_from_wild_battles = TRUE;
   
   gCustomGameOptions.battle.scale_outgoing_damage.by_player = 100;
   gCustomGameOptions.battle.scale_outgoing_damage.by_enemy  = 100;
   gCustomGameOptions.battle.scale_outgoing_damage.by_ally   = 100;
   
   gCustomGameOptions.battle.scale_outgoing_accuracy.player = 100;
   gCustomGameOptions.battle.scale_outgoing_accuracy.enemy  = 100;
   gCustomGameOptions.battle.scale_outgoing_accuracy.ally   = 100;
   
   gCustomGameOptions.battle.scale_exp.received.normal = 100;
   gCustomGameOptions.battle.scale_exp.received.traded = 150;
   gCustomGameOptions.battle.scale_exp.awarded.trainer_battles = 150;
   gCustomGameOptions.battle.scale_exp.awarded.wild_battles = 100;
   
   gCustomGameOptions.events.eon_ticket = CGO_EONTICKETMODE_VANILLA;
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
   gCustomGameOptions.starters.force_gender = CGO_STARTERPOKEMONGENDER_RANDOM;
   gCustomGameOptions.starters.level = 5;
   
   gCustomGameOptions.wild_encounters.scale_encounter_rate = 100;
}

void ResetCustomGameSavestate(void) {
}

extern void SetCustomGameOptionsPerGeneration(struct CustomGameOptionsSet* dst, u8 generation) {
   if (generation >= 4) {
      gCustomGameOptions.can_run_indoors = TRUE;
      
      gCustomGameOptions.battle.money.modern_calc_loss_on_defeat = TRUE;
      
      gCustomGameOptions.overworld_poison.faint = FALSE;
      
      if (generation >= 5) {
         gCustomGameOptinos.infinite_use_tms = TRUE;
         gCustomGameOptions.overworld_poison.interval = 0;
         gCustomGameOptions.overworld_poison.damage   = 0;
         if (generation >= 6) {
            gCustomGameOptions.battle.catching.enable_catch_exp = TRUE;
            if (generation >= 7) {
               gCustomGameOptions.battle.scale_exp.awarded.trainer_battles = 100;
            }
         }
      }
   }
}

#include "global.h"
#include "event_data.h"

void CustomGames_HandleNewPlaythroughStarted(void) {
   ResetCustomGameSavestate();
   
   if (gCustomGameOptions.start_with_running_shoes) {
      FlagSet(FLAG_SYS_B_DASH);
   }
}