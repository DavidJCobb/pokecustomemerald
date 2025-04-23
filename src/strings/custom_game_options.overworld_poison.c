#include "strings/custom_game_options.overworld_poison.h"

const u8 gText_lu_CGOptionCategoryName_OverworldPoison[] = _("Overworld poison damage");
const u8 gText_lu_CGOptionCategoryHelp_OverworldPoison[] = _("On the overworld, poisoned Pokémon take damage after every few steps you take. You can change the number of steps and amount of damage dealt, or disable poison damage on the overworld entirely.");

const u8 gText_lu_CGOptionName_OverworldPoison_Interval[] = _("Damage interval (steps)");
const u8 gText_lu_CGOptionHelp_OverworldPoison_Interval[] = _("Poisoned Pokémon take damage after every few steps you take. (Steps are counted even when no one is poisoned.) You can change the number of steps, or disable poison damage on the overworld entirely.\n\nDefault: Pokémon take damage every 4 steps.");

const u8 gText_lu_CGOptionName_OverworldPoison_Damage[] = _("Damage dealt");
const u8 gText_lu_CGOptionHelp_OverworldPoison_Damage[] = _("Poisoned Pokémon take damage after every few steps you take. You can change the amount of damage dealt at one time.\n\nDefault: Pokémon take 1 HP of damage at a time.");

const u8 gText_lu_CGOptionName_OverworldPoison_Termination[] = _("Allow Pokémon to faint");
const u8 gText_lu_CGOptionHelp_OverworldPoison_Termination[] = _("Poisoned Pokémon take damage after every few steps you take. You can control whether Pokémon may faint as a result, or whether they survive and are cured upon reaching 1 HP.\n\nDefault: Pokémon can faint from being poisoned.");
const u8 gText_lu_CGOptionHelp_OverworldPoison_Termination_Faint[] = _("Faint at 0 HP");
const u8 gText_lu_CGOptionHelp_OverworldPoison_Termination_Survive[] = _("Cure at 1 HP");