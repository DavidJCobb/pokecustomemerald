#include "strings/custom_game_options.battle.h"

const u8 gText_lu_CGOptionCategoryName_Battles[] = _("Battle options");

const u8 gText_lu_CGOptionName_ItemUseInBattles[] = _("Item use");
const u8 gText_lu_CGOptionHelp_ItemUseInBattles[] = _("Control whether the player is allowed to use items while in battle.");
//
const u8 gText_lu_CGOptionValueName_ItemUseInBattles_NoBackfield[] = _("No Backfield");
const u8 gText_lu_CGOptionValueHelp_ItemUseInBattles_NoBackfield[] = _("Control whether the player is allowed to use items while in battle. The “No Backfield” option limits players to using items on Pokémon that are currently in battle; this is the same limit that NPCs have. This limit does not apply to items that can revive a fainted Pokémon; to disable those too, choose “NB + NR”.");
const u8 gText_lu_CGOptionValueName_ItemUseInBattles_NoRevives[] = _("No Revives");
const u8 gText_lu_CGOptionValueHelp_ItemUseInBattles_NoRevives[] = _("Control whether the player is allowed to use items while in battle. The “No Revives” option disallows reviving a fainted Pokémon.");
const u8 gText_lu_CGOptionValueName_ItemUseInBattles_NoBackfieldAndNoRevives[] = _("NB + NR");
const u8 gText_lu_CGOptionValueHelp_ItemUseInBattles_NoBackfieldAndNoRevives[] = _("Control whether the player is allowed to use items while in battle. The “NB + NR” option doesn't allow the player to use items on Pokémon that aren't on the field, even revives.");

const u8 gText_lu_CGOptionName_BattlesScaleDamagePlayer[] = _("Scale damage (player)");
const u8 gText_lu_CGOptionHelp_BattlesScaleDamagePlayer[] = _("Applies a percentage multiplier to damage dealt by the player. This multiplier is applied after all other damage calculations, and does not affect NPC AI decision-making.");
const u8 gText_lu_CGOptionName_BattlesScaleDamageEnemy[] = _("Scale damage (enemies)");
const u8 gText_lu_CGOptionHelp_BattlesScaleDamageEnemy[] = _("Applies a percentage multiplier to damage dealt by the player's opponents. This multiplier is applied after all other damage calculation, and does not affect NPC AI decision-making.");
const u8 gText_lu_CGOptionName_BattlesScaleDamageAlly[] = _("Scale damage (allies)");
const u8 gText_lu_CGOptionHelp_BattlesScaleDamageAlly[] = _("Applies a percentage multiplier to damage dealt by the player's NPC allies, like Steven in Mossdeep City. This multiplier is applied after all other damage calculations, and does not affect NPC AI decision-making.");

const u8 gText_lu_CGOptionName_BattlesScaleAccuracyPlayer[] = _("Scale accuracy (player)");
const u8 gText_lu_CGOptionHelp_BattlesScaleAccuracyPlayer[] = _("Applies a percentage multiplier to the player's accuracy. This multiplier is applied after all other accuracy calculations, and does not affect NPC AI decision-making.");
const u8 gText_lu_CGOptionName_BattlesScaleAccuracyEnemy[] = _("Scale accuracy (enemies)");
const u8 gText_lu_CGOptionHelp_BattlesScaleAccuracyEnemy[] = _("Applies a percentage multiplier to opponents' accuracy. This multiplier is applied after all other accuracy calculations, and does not affect NPC AI decision-making.");
const u8 gText_lu_CGOptionName_BattlesScaleAccuracyAlly[] = _("Scale accuracy (allies)");
const u8 gText_lu_CGOptionHelp_BattlesScaleAccuracyAlly[] = _("Applies a percentage multiplier to NPC allies' accuracy. This multiplier is applied after all other accuracy calculations, and does not affect NPC AI decision-making.");

const u8 gText_lu_CGOptionName_BattlesScaleEXPNormal[] = _("Scale EXP gains (normal)");
const u8 gText_lu_CGOptionHelp_BattlesScaleEXPNormal[] = _("Scale the EXP awarded to the player's non-traded Pokémon.\n\nDefault: 100%.");
const u8 gText_lu_CGOptionName_BattlesScaleEXPTraded[] = _("Scale EXP gains (traded)");
const u8 gText_lu_CGOptionHelp_BattlesScaleEXPTraded[] = _("Scale the EXP awarded to the player's traded Pokémon. Text in the UI will display as normal, describing the EXP earned as “boosted” regardless of the value you set here.\n\nDefault: 150%.");
const u8 gText_lu_CGOptionName_BattlesScaleEXPTrainerBattles[] = _("Scale EXP gains (trainer battles)");
const u8 gText_lu_CGOptionHelp_BattlesScaleEXPTrainerBattles[] = _("Scale the EXP awarded to the player's Pokémon during trainer battles. In all games up to Gen 7, trainer battles scaled EXP gain by 150%.\n\nDefault: 150%.");

const u8 gText_lu_CGOptionName_BattlesScaleVictoryPayout[] = _("Money gain on victory");
const u8 gText_lu_CGOptionHelp_BattlesScaleVictoryPayout[] = _("Applies a percentage multiplier to the money the player earns after winning a battle. This only affects the payout received from the opposing trainer; it doesn't affect other income, such as the results of using Pay Day.");

const u8 gText_lu_CGOptionName_MoneyLossOnDefeat[] = _("Money loss on defeat");
const u8 gText_lu_CGOptionHelp_MoneyLossOnDefeat[] = _("In the classic Pokémon games, losing a battle would cost the player exactly half of all the money they were carrying. In FireRed, LeafGreen, and from Gen IV onward, the player's money loss is calculated similarly to NPCs.\n\nDefault: Classic.");
//
const u8 gText_lu_CGOptionValueName_MoneyLossOnDefeat_Classic[] = _("Classic");
const u8 gText_lu_CGOptionValueName_MoneyLossOnDefeat_Modern[] = _("Modern");