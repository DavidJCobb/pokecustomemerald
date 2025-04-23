#ifndef GUARD_LU_CUSTOM_GAME_OPTIONS
#define GUARD_LU_CUSTOM_GAME_OPTIONS

#include "lu/bitpack_options.h"
#include "gba/types.h"
#include "gba/defines.h"

enum {
   CustomGame_ItemUseInBattles_Enabled,
   CustomGame_ItemUseInBattles_NoBackfield,
   CustomGame_ItemUseInBattles_NoRevives,
   CustomGame_ItemUseInBattles_NoBackfieldAndNoRevives,
   CustomGame_ItemUseInBattles_Disabled,
};
LU_BP_MINMAX(0, 4) typedef u8 CustomGameItemUseInBattlesMode;

enum {
   CustomGame_PlayerStarterForceGender_Random,
   CustomGame_PlayerStarterForceGender_Male,
   CustomGame_PlayerStarterForceGender_Female,
};

LU_BP_MINMAX(0, 5000) typedef u16 CustomGameScalePct; // 100 = 100%

struct CustomGameScaleAndClamp {
   CustomGameScalePct scale;
   u16 min;
   u16 max; // 0xFFFF = no max
};

u16 ApplyCustomGameScaleAndClamp_u16(u16, const struct CustomGameScaleAndClamp*);

u16 ApplyCustomGameScale_u16(u16, CustomGameScalePct scale);
u32 ApplyCustomGameScale_u32(u32, CustomGameScalePct scale);
s32 ApplyCustomGameScale_s32(s32, CustomGameScalePct scale);

#define UNIMPLEMENTED_CUSTOM_GAME_OPTION
#define UNTESTED_CUSTOM_GAME_OPTION

// Track current values of Custom Game options. Intended to be serialized after SaveBlock2.
extern struct CustomGameOptions {
   bool8 start_with_running_shoes;
   bool8 can_run_indoors;
   bool8 can_bike_indoors;
   
   CustomGameScalePct scale_wild_encounter_rate;
   
   bool8 enable_catch_exp;
   
   CustomGameScalePct catch_rate_scale;
   LU_BP_MINMAX(0, 100) u8 catch_rate_increase_base;
   
   CustomGameItemUseInBattlesMode battle_item_usage;
   struct {
      CustomGameScalePct by_player;
      CustomGameScalePct by_enemy;
      CustomGameScalePct by_ally;
   } scale_battle_damage;
   struct {
      CustomGameScalePct player;
      CustomGameScalePct enemy;
      CustomGameScalePct ally;
   } scale_battle_accuracy;
   struct {
      CustomGameScalePct normal;
      CustomGameScalePct traded;
   } scale_exp_gains;
   CustomGameScalePct scale_player_money_gain_on_victory;
   bool8 modern_calc_player_money_loss_on_defeat;
   
   struct {
      LU_BP_MINMAX(0, 60)   u8    interval;
      LU_BP_MINMAX(1, 2000) u16   damage;
      LU_BP_DEFAULT(TRUE)   bool8 faint;
   } overworld_poison;
   
   struct {
      LU_BP_DEFAULT(0) PokemonSpeciesID species[3];
      LU_BP_DEFAULT(0) u8 forceGender;
      LU_BP_DEFAULT(5) PokemonLevel level;
   } starters;
   
} gCustomGameOptions;

// Track in-game progress related to custom game options. Intended to be serialized 
// after SaveBlock2.
//
// Needed for Nuzlocke encounter options: we should not enforce any encounter/catch 
// limits until the player has obtained at least one of any Poke Ball type.
extern struct CustomGameSavestate {
} gCustomGameSavestate;

#undef UNIMPLEMENTED_CUSTOM_GAME_OPTION
#undef UNTESTED_CUSTOM_GAME_OPTION

extern void ResetCustomGameOptions(void);
extern void ResetCustomGameSavestate(void);

extern void CustomGames_HandleNewPlaythroughStarted(void);

#endif