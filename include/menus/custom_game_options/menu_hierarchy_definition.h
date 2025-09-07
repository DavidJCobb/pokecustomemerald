#include "menus/custom_game_options/menu_item.h"
#include "lu/strings.h"

#include "strings/custom_game_options.battle.h"
#include "strings/custom_game_options.daycare_eggs.h"
#include "strings/custom_game_options.events.h"
#include "strings/custom_game_options.mirage_island.h"
#include "strings/custom_game_options.overworld_poison.h"
#include "strings/custom_game_options.rematch.h"

#include "custom_game_options/value_types/battle_item_usage.h"
#include "custom_game_options/value_types/eon_ticket_mode.h"
#include "custom_game_options/value_types/starter_pokemon_gender.h"

#define END_OF_LIST_SENTINEL { \
   .name        = NULL, \
   .help_string = NULL, \
   .flags       = (1 << MENUITEM_FLAG_END_OF_LIST_SENTINEL), \
   .value_type  = VALUE_TYPE_NONE, \
   .target      = NULL, \
}

#define END_OF_NAME_OVERRIDES_SENTINEL { .name = NULL }

//

static const struct CGOptionMenuItem sCatchingOptions[] = {
   {  // Catch EXP
      .name        = gText_lu_CGOptionName_CatchEXP,
      .help_string = gText_lu_CGOptionHelp_CatchEXP,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.battle.catching.enable_catch_exp
      }
   },
   {  // Base catch rate
      .name        = gText_lu_CGOptionName_CatchRateBase,
      .help_string = gText_lu_CGOptionHelp_CatchRateBase,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U8,
      .values = {
         .integral = {
            .min = 0,
            .max = 100,
         }
      },
      .target = {
         .as_u8 = &sTempOptions.battle.catching.catch_rate_increase_base
      }
   },
   {  // Scale catch rate
      .name        = gText_lu_CGOptionName_CatchRateScale,
      .help_string = gText_lu_CGOptionHelp_CatchRateScale,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.catching.catch_rate_scale
      }
   },
   END_OF_LIST_SENTINEL,
};

//

static const struct CGOptionMenuItem sRematchOptions[] = {
   {  // Min badge count
      .name        = gText_lu_CGOptionName_Rematch_MinBadges,
      .help_string = gText_lu_CGOptionHelp_Rematch_MinBadges,
      .value_type = VALUE_TYPE_U8,
      .values = {
         .integral = {
            .min = 0,
            .max = 8,
         }
      },
      .target = {
         .as_u8 = &sTempOptions.rematches.min_badge_count
      }
   },
   {  // Chance
      .name        = gText_lu_CGOptionName_Rematch_Chance,
      .help_string = gText_lu_CGOptionHelp_Rematch_Chance,
      .value_type = VALUE_TYPE_U8,
      .values = {
         .integral = {
            .min = 0,
            .max = 100,
         }
      },
      .target = {
         .as_u8 = &sTempOptions.rematches.chance
      }
   },
   {  // Interval
      .name        = gText_lu_CGOptionName_Rematch_Interval,
      .help_string = gText_lu_CGOptionHelp_Rematch_Interval,
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.rematches.interval
      }
   },
   END_OF_LIST_SENTINEL,
};

//

static const u8* const sOption_ItemUseInBattles_ValueNameStrings[] = {
   gText_lu_CGOptionValues_common_Enabled,
   gText_lu_CGOptionValueName_ItemUseInBattles_NoBackfield,
   gText_lu_CGOptionValueName_ItemUseInBattles_NoRevives,
   gText_lu_CGOptionValueName_ItemUseInBattles_NoBackfieldAndNoRevives,
   gText_lu_CGOptionValues_common_Disabled,
};   
static const u8* const sOption_ItemUseInBattles_ValueHelpStrings[] = {
   NULL,
   gText_lu_CGOptionValueHelp_ItemUseInBattles_NoBackfield,
   gText_lu_CGOptionValueHelp_ItemUseInBattles_NoRevives,
   gText_lu_CGOptionValueHelp_ItemUseInBattles_NoBackfieldAndNoRevives,
   NULL,
};
//
static const u8* const sOption_MoneyLossOnDefeat_ValueNameStrings[] = {
   gText_lu_CGOptionValueName_MoneyLossOnDefeat_Classic,
   gText_lu_CGOptionValueName_MoneyLossOnDefeat_Modern,
};   
static const u8* const sOption_MoneyLossOnDefeat_ValueHelpStrings[] = {
   NULL,
   NULL,
};
//
static const struct CGOptionMenuItem sBattleOptions[] = {
   {  // Item use
      .name        = gText_lu_CGOptionName_ItemUseInBattles,
      .help_string = gText_lu_CGOptionHelp_ItemUseInBattles,
      .flags       = (1 << MENUITEM_FLAG_IS_ENUM),
      .value_type = VALUE_TYPE_U8,
      .values = {
         .named = {
            .name_strings = sOption_ItemUseInBattles_ValueNameStrings,
            .help_strings = sOption_ItemUseInBattles_ValueHelpStrings,
            .count = 5,
         }
      },
      .target = {
         .as_u8 = (u8*)&sTempOptions.battle.item_usage
      }
   },
   {  // Scale accuracy by player
      .name        = gText_lu_CGOptionName_BattlesScaleAccuracyPlayer,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleAccuracyPlayer,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
            .formatting = NULL,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.scale_outgoing_accuracy.player
      }
   },
   {  // Scale accuracy by enemy
      .name        = gText_lu_CGOptionName_BattlesScaleAccuracyEnemy,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleAccuracyEnemy,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.scale_outgoing_accuracy.enemy
      }
   },
   {  // Scale accuracy by ally
      .name        = gText_lu_CGOptionName_BattlesScaleAccuracyAlly,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleAccuracyAlly,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.scale_outgoing_accuracy.ally
      }
   },
   {  // Scale damage by player
      .name        = gText_lu_CGOptionName_BattlesScaleDamagePlayer,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleDamagePlayer,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.scale_outgoing_damage.by_player
      }
   },
   {  // Scale damage by enemy
      .name        = gText_lu_CGOptionName_BattlesScaleDamageEnemy,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleDamageEnemy,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.scale_outgoing_damage.by_enemy
      }
   },
   {  // Scale damage by ally
      .name        = gText_lu_CGOptionName_BattlesScaleDamageAlly,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleDamageAlly,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.scale_outgoing_damage.by_ally
      }
   },
   {  // Scale EXP gains (normal)
      .name        = gText_lu_CGOptionName_BattlesScaleEXPNormal,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleEXPNormal,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.scale_exp.received.normal
      }
   },
   {  // Scale EXP gains (traded)
      .name        = gText_lu_CGOptionName_BattlesScaleEXPTraded,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleEXPTraded,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.scale_exp.received.traded
      }
   },
   {  // Scale EXP gains (trainer battles)
      .name        = gText_lu_CGOptionName_BattlesScaleEXPTrainerBattles,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleEXPTrainerBattles,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.scale_exp.awarded.trainer_battles
      }
   },
   {  // Scale EXP gains (wild battles)
      .name        = gText_lu_CGOptionName_BattlesScaleEXPWildBattles,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleEXPWildBattles,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.scale_exp.awarded.wild_battles
      }
   },
   {  // Scale monetary earnings on victory
      .name        = gText_lu_CGOptionName_BattlesScaleVictoryPayout,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleVictoryPayout,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.battle.money.scale_gain_on_victory
      }
   },
   {  // Use modern calc for money loss on defeat
      .name        = gText_lu_CGOptionName_MoneyLossOnDefeat,
      .help_string = gText_lu_CGOptionHelp_MoneyLossOnDefeat,
      .flags       = (1 << MENUITEM_FLAG_IS_ENUM),
      .value_type = VALUE_TYPE_BOOL8,
      .values = {
         .named = {
            .name_strings = sOption_MoneyLossOnDefeat_ValueNameStrings,
            .help_strings = sOption_MoneyLossOnDefeat_ValueHelpStrings,
            .count = 2,
         }
      },
      .target = {
         .as_bool8 = &sTempOptions.battle.money.modern_calc_loss_on_defeat
      }
   },
   {  // Never lose money from wild battles
      .name        = gText_lu_CGOptionName_MoneyLossWild,
      .help_string = gText_lu_CGOptionHelp_MoneyLossWild,
      .flags       = 0,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.battle.money.lose_money_from_wild_battles
      }
   },
   {  // SUBMENU: Rematch
      .name        = gText_lu_CGOptionCategoryName_Rematch,
      .help_string = NULL,
      .flags       = (1 << MENUITEM_FLAG_IS_SUBMENU),
      .value_type = VALUE_TYPE_NONE,
      .target = {
         .submenu = sRematchOptions
      },
   },
   END_OF_LIST_SENTINEL,
};

//

static const struct CGOptionMenuItem sDaycareEggsOptions[] = {
   {  // Daycare can teach moves
      .name        = gText_lu_CGOptionName_DaycareEggs_DaycareCanTeachMoves,
      .help_string = gText_lu_CGOptionHelp_DaycareEggs_DaycareCanTeachMoves,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.daycare_eggs.can_teach_moves
      }
   },
   {  // Daycare scale cost
      .name        = gText_lu_CGOptionName_DaycareEggs_DaycareScaleCost,
      .help_string = gText_lu_CGOptionHelp_DaycareEggs_DaycareScaleCost,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.daycare_eggs.scale_cost
      }
   },
   {  // Daycare scale EXP
      .name        = gText_lu_CGOptionName_DaycareEggs_DaycareScaleEXP,
      .help_string = gText_lu_CGOptionHelp_DaycareEggs_DaycareScaleEXP,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.daycare_eggs.scale_step_exp
      }
   },
   {  // Egg lay chance
      .name        = gText_lu_CGOptionName_DaycareEggs_EggLayChance,
      .help_string = gText_lu_CGOptionHelp_DaycareEggs_EggLayChance,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 500,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.daycare_eggs.egg_lay_chance
      }
   },
   END_OF_LIST_SENTINEL,
};

//

static const struct CGOptionMenuItemIntegralValueNameOverride sMirageIslandRarityOverrides[] = {
   {
      .name  = gText_lu_CGOptionValues_common_Always,
      .value = 0,
   },
   {
      .name  = gText_lu_CGOptionValues_common_Never,
      .value = 17,
   },
   END_OF_NAME_OVERRIDES_SENTINEL
};
static const struct CGOptionMenuItemIntegralFormat sMirageIslandRarityFmt = {
   .format_string  = gText_lu_CGOptionValueFormat_MirageIsland_Rarity,
   .name_overrides = sMirageIslandRarityOverrides,
};
//
static const struct CGOptionMenuItem sEventsMirageIslandOptions[] = {
   {  // Include boxed Pokemon
      .name        = gText_lu_CGOptionName_MirageIsland_IncludePC,
      .help_string = gText_lu_CGOptionHelp_MirageIsland_IncludePC,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.events.mirage_island.include_pc
      }
   },
   {  // Rarity
      .name        = gText_lu_CGOptionName_MirageIsland_Rarity,
      .help_string = gText_lu_CGOptionHelp_MirageIsland_Rarity,
      .value_type = VALUE_TYPE_U8,
      .values = {
         .integral = {
            .min = 0,
            .max = 17,
            .formatting = &sMirageIslandRarityFmt,
         }
      },
      .target = {
         .as_u8 = &sTempOptions.events.mirage_island.rarity
      }
   },
   END_OF_LIST_SENTINEL,
};

static const u8* const sOption_Events_EonTicket_ValueNameStrings[] = {
   gText_lu_CGOptionValues_common_Disabled,
   gText_lu_CGOptionValues_common_Enabled,
   gText_lu_CGOptionValueName_Events_EonTicket_AfterRoamer,
};   
static const u8* const sOption_Events_EonTicket_ValueHelpStrings[] = {
   NULL,
   NULL,
   gText_lu_CGOptionValueHelp_Events_EonTicket_AfterRoamer,
};

static const struct CGOptionMenuItem sEventOptions[] = {
   {  // Eon Ticket
      .name        = gText_lu_CGOptionName_Events_EonTicket,
      .help_string = gText_lu_CGOptionHelp_Events_EonTicket,
      .flags       = (1 << MENUITEM_FLAG_IS_ENUM),
      .value_type  = VALUE_TYPE_U8,
      .values = {
         .named = {
            .name_strings = sOption_Events_EonTicket_ValueNameStrings,
            .help_strings = sOption_Events_EonTicket_ValueHelpStrings,
            .count = 3,
         }
      },
      .target = {
         .as_u8 = (u8*)&sTempOptions.events.eon_ticket
      }
   },
   {  // SUBMENU: Mirage Island
      .name        = gText_lu_CGOptionCategoryName_MirageIsland,
      .help_string = NULL,
      .flags       = (1 << MENUITEM_FLAG_IS_SUBMENU),
      .value_type = VALUE_TYPE_NONE,
      .target = {
         .submenu = sEventsMirageIslandOptions
      }
   },
   END_OF_LIST_SENTINEL,
};

//

static const u8* const sOption_OverworldPoison_Faint_ValueNameStrings[] = {
   gText_lu_CGOptionHelp_OverworldPoison_Termination_Survive,
   gText_lu_CGOptionHelp_OverworldPoison_Termination_Faint,
};   
static const u8* const sOption_OverworldPoison_Faint_ValueHelpStrings[] = {
   NULL,
   NULL,
};

static const struct CGOptionMenuItem sOverworldPoisonOptions[] = {
   {  // Interval
      .name        = gText_lu_CGOptionName_OverworldPoison_Interval,
      .help_string = gText_lu_CGOptionHelp_OverworldPoison_Interval,
      .flags       = (1 << MENUITEM_FLAG_0_MEANS_DISABLED),
      .value_type = VALUE_TYPE_U8,
      .values = {
         .integral = {
            .min = 0,
            .max = 60,
         }
      },
      .target = {
         .as_u8 = &sTempOptions.overworld_poison.interval
      }
   },
   {  // Damage
      .name        = gText_lu_CGOptionName_OverworldPoison_Damage,
      .help_string = gText_lu_CGOptionHelp_OverworldPoison_Damage,
      .flags       = 0,
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 1,
            .max = 2000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.overworld_poison.damage
      }
   },
   {  // Faint or cure
      .name        = gText_lu_CGOptionName_OverworldPoison_Termination,
      .help_string = gText_lu_CGOptionHelp_OverworldPoison_Termination,
      .flags       = (1 << MENUITEM_FLAG_IS_ENUM),
      .value_type  = VALUE_TYPE_BOOL8,
      .values = {
         .named = {
            .name_strings = sOption_OverworldPoison_Faint_ValueNameStrings,
            .help_strings = sOption_OverworldPoison_Faint_ValueHelpStrings,
            .count = 2,
         }
      },
      .target = {
         .as_bool8 = &sTempOptions.overworld_poison.faint
      }
   },
   END_OF_LIST_SENTINEL,
};

static const u8* const sOption_Starters_Gender_ValueNameStrings[] = {
   gText_lu_CGOptionValues_common_Random,
   gText_lu_CGOptionValueName_Starters_Gender_Male,
   gText_lu_CGOptionValueName_Starters_Gender_Female,
};   
static const u8* const sOption_Starters_Gender_ValueHelpStrings[] = {
   NULL,
   NULL,
};
//
static const struct CGOptionMenuItem sStarterPokemonOptions[] = {
   {  // Species
      .name        = gText_lu_CGOptionName_Starters_Species_0,
      .help_string = gText_lu_CGOptionHelp_Starters_Species_0,
      .flags       = (1 << MENUITEM_FLAG_0_MEANS_DEFAULT) | (1 << MENUITEM_FLAG_POKEMON_SPECIES_ALLOW_0),
      .value_type = VALUE_TYPE_POKEMON_SPECIES,
      .target = {
         .as_u16 = &sTempOptions.starters.species[0]
      }
   },
   {  // Species
      .name        = gText_lu_CGOptionName_Starters_Species_1,
      .help_string = gText_lu_CGOptionHelp_Starters_Species_1,
      .flags       = (1 << MENUITEM_FLAG_0_MEANS_DEFAULT) | (1 << MENUITEM_FLAG_POKEMON_SPECIES_ALLOW_0),
      .value_type = VALUE_TYPE_POKEMON_SPECIES,
      .target = {
         .as_u16 = &sTempOptions.starters.species[1]
      }
   },
   {  // Species
      .name        = gText_lu_CGOptionName_Starters_Species_2,
      .help_string = gText_lu_CGOptionHelp_Starters_Species_2,
      .flags       = (1 << MENUITEM_FLAG_0_MEANS_DEFAULT) | (1 << MENUITEM_FLAG_POKEMON_SPECIES_ALLOW_0),
      .value_type = VALUE_TYPE_POKEMON_SPECIES,
      .target = {
         .as_u16 = &sTempOptions.starters.species[2]
      }
   },
   {  // Gender
      .name        = gText_lu_CGOptionName_Starters_Gender,
      .help_string = gText_lu_CGOptionHelp_Starters_Gender,
      .flags       = (1 << MENUITEM_FLAG_IS_ENUM),
      .value_type = VALUE_TYPE_U8,
      .values = {
         .named = {
            .name_strings = sOption_Starters_Gender_ValueNameStrings,
            .help_strings = sOption_Starters_Gender_ValueHelpStrings,
            .count = 3,
         }
      },
      .target = {
         .as_u8 = (u8*)&sTempOptions.starters.force_gender
      }
   },
   {  // Level
      .name        = gText_lu_CGOptionName_Starters_Level,
      .help_string = gText_lu_CGOptionHelp_Starters_Level,
      .flags       = 0,
      .value_type = VALUE_TYPE_U8,
      .values = {
         .integral = {
            .min = 1,
            .max = 100,
         }
      },
      .target = {
         .as_u8 = &sTempOptions.starters.level
      }
   },
   END_OF_LIST_SENTINEL,
};

static const struct CGOptionMenuItem sTopLevelMenu[] = {
   {  // Start with running shoes
      .name        = gText_lu_CGOptionName_StartWithRunningShoes,
      .help_string = gText_lu_CGOptionHelp_StartWithRunningShoes,
      .flags       = 0,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.start_with_running_shoes
      }
   },
   {  // Allow running indoors
      .name        = gText_lu_CGOptionName_AllowRunningIndoors,
      .help_string = NULL,
      .flags       = 0,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.can_run_indoors
      }
   },
   {  // Allow biking indoors
      .name        = gText_lu_CGOptionName_AllowBikingIndoors,
      .help_string = NULL,
      .flags       = 0,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.can_bike_indoors
      }
   },
   {  // Infinite-use TMs
      .name        = gText_lu_CGOptionName_InfiniteUseTMs,
      .help_string = gText_lu_CGOptionHelp_InfiniteUseTMs,
      .flags       = 0,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.infinite_use_tms
      }
   },
   {  // SUBMENU: Battle options
      .name        = gText_lu_CGOptionCategoryName_Battles,
      .help_string = NULL,
      .flags       = (1 << MENUITEM_FLAG_IS_SUBMENU),
      .value_type = VALUE_TYPE_NONE,
      .target = {
         .submenu = sBattleOptions
      },
   },
   {  // SUBMENU: Catching options
      .name        = gText_lu_CGOptionCategoryName_Catching,
      .help_string = NULL,
      .flags       = (1 << MENUITEM_FLAG_IS_SUBMENU),
      .value_type = VALUE_TYPE_NONE,
      .target = {
         .submenu = sCatchingOptions
      },
   },
   {  // SUBMENU: Daycare and eggs
      .name        = gText_lu_CGOptionCategoryName_DaycareEggs,
      .help_string = NULL,
      .flags       = (1 << MENUITEM_FLAG_IS_SUBMENU),
      .value_type = VALUE_TYPE_NONE,
      .target = {
         .submenu = sDaycareEggsOptions
      },
   },
   {  // SUBMENU: Event options
      .name        = gText_lu_CGOptionCategoryName_Events,
      .help_string = NULL,
      .flags       = (1 << MENUITEM_FLAG_IS_SUBMENU),
      .value_type = VALUE_TYPE_NONE,
      .target = {
         .submenu = sEventOptions
      },
   },
   {  // SUBMENU: Overworld poison
      .name        = gText_lu_CGOptionCategoryName_OverworldPoison,
      .help_string = gText_lu_CGOptionCategoryHelp_OverworldPoison,
      .flags       = (1 << MENUITEM_FLAG_IS_SUBMENU),
      .value_type = VALUE_TYPE_NONE,
      .target = {
         .submenu = sOverworldPoisonOptions
      }
   },
   {  // SUBMENU: Starter species
      .name        = gText_lu_CGOptionCategoryName_StarterPokemon,
      .help_string = gText_lu_CGOptionCategoryHelp_StarterPokemon,
      .flags       = (1 << MENUITEM_FLAG_IS_SUBMENU),
      .value_type = VALUE_TYPE_NONE,
      .target = {
         .submenu = sStarterPokemonOptions
      }
   },
   END_OF_LIST_SENTINEL,
};
