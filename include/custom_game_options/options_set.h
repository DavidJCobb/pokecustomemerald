#ifndef GUARD_CGO_OPTIONS_SET
#define GUARD_CGO_OPTIONS_SET

#include "lu/c.h"
#include "lu/bitpack_options.h"
#include "lu/game_typedefs.h"
#include "gba/defines.h"

#include "./value_types/battle_item_usage.h"
#include "./value_types/eon_ticket_mode.h"
#include "./value_types/scale_and_clamp.h"
#include "./value_types/scale_percentage.h"
#include "./value_types/starter_pokemon_gender.h"

struct CustomGameOptionsSet {
   bool8 start_with_running_shoes;
   bool8 can_run_indoors;
   bool8 can_bike_indoors;
   
   struct {
      struct {
         bool8                   enable_catch_exp;
         CustomGameScalePct      catch_rate_scale;
         LU_BP_MINMAX(0, 100) u8 catch_rate_increase_base;
      } catching;
      enum CGO_BattleItemUsage item_usage;
      struct {
         CustomGameScalePct scale_gain_on_victory;
         bool8              modern_calc_loss_on_defeat;
         bool8              lose_money_from_wild_battles;
      } money;
      struct {
         CustomGameScalePct by_player;
         CustomGameScalePct by_enemy;
         CustomGameScalePct by_ally;
      } scale_outgoing_damage;
      struct {
         CustomGameScalePct player;
         CustomGameScalePct enemy;
         CustomGameScalePct ally;
      } scale_outgoing_accuracy;
      struct {
         struct {
            LU_BP_DEFAULT(150) CustomGameScalePct trainer_battles;
            LU_BP_DEFAULT(100) CustomGameScalePct wild_battles;
         } awarded;
         struct {
            CustomGameScalePct normal;
            CustomGameScalePct traded;
         } received;
      } scale_exp;
   } battle;
   struct {
      LU_BP_DEFAULT(TRUE) bool8 can_teach_moves;
      LU_BP_DEFAULT(100)  CustomGameScalePct scale_cost;
      LU_BP_DEFAULT(100)  CustomGameScalePct scale_step_exp;
      LU_BP_DEFAULT(100) LU_BP_MINMAX(0, 500) CustomGameScalePct egg_lay_chance;
   } daycare_eggs;
   struct {
      enum CGO_EonTicketMode eon_ticket;
      struct {
         LU_BP_DEFAULT(16) LU_BP_MINMAX(0, 16 + 1) u8 rarity; // +1 for Never
         LU_BP_DEFAULT(FALSE) bool8 include_pc;
      } mirage_island;
   } events;
   struct {
      LU_BP_MINMAX(0, 60)   LU_BP_DEFAULT(4) u8    interval;
      LU_BP_MINMAX(1, 2000) LU_BP_DEFAULT(1) u16   damage;
      LU_BP_DEFAULT(TRUE)                    bool8 faint;
   } overworld_poison;
   struct {
      LU_BP_DEFAULT(5)   LU_BP_MINMAX(0,    8) u8  min_badge_count;
      LU_BP_DEFAULT(31)  LU_BP_MINMAX(0,  100) u8  chance;
      LU_BP_DEFAULT(255) LU_BP_MINMAX(0, 5000) u16 interval;
   } rematches;
   struct {
      LU_BP_DEFAULT(0) PokemonSpeciesID species[3];
      enum CGO_StarterPokemonGender force_gender;
      LU_BP_DEFAULT(5) PokemonLevel level;
   } starters;
   struct {
      CustomGameScalePct scale_encounter_rate;
   } wild_encounters;
};

#endif