#include "global.h"
#include "lu/strings.h"

// Options Menu: Option names
const u8 gText_lu_option_running[] = _("RUNNING");

// Options Menu: option value labels
// Code for this menu assumes that the strings begin with two format codes. It first 
// copies the strings to a buffer; then overwrites the format codes to distinguish 
// the selected value from the others. Option values have a maximum length of 16, 
// including the format codes (which are each multiple bytes) and terminator -- so, 
// a maximum text length of 11 or so.
const u8 gText_lu_option_running_vanilla[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}HOLD");
const u8 gText_lu_option_running_toggle[]  = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}TOGGLE");

//

const u8 gText_lu_MainMenuCustomGameOptions[] = _("CUSTOM GAME OPTIONS");

//
// NEW GAME SUB-OPTIONS
//

const u8 gText_lu_NewGame_Vanilla[] = _("Vanilla Options");
const u8 gText_lu_NewGame_Enhanced[] = _("Enhanced Options");
const u8 gText_lu_NewGame_CustomGame[] = _("Custom Game");

//
// MESSAGES
//

const u8 gText_lu_CGRevivesNotAllowedInBattle[] = ("Can't use revives in battle.");

//
// CUSTOM GAME OPTIONS MENU
//

const u8 gText_lu_UI_KeybindBack[]          = _("BACK");   // back out of a (sub)menu
const u8 gText_lu_UI_KeybindChange[]        = _("CHANGE"); // modify the value of something
const u8 gText_lu_UI_KeybindChooseSpecies[] = _("CHOOSE SPECIES"); // confirm selection of a species
const u8 gText_lu_UI_KeybindEnterSubmenu[]  = _("ENTER SUBMENU");
const u8 gText_lu_UI_KeybindHelp[]          = _("HELP");   // open a help screen
const u8 gText_lu_UI_KeybindPick[]          = _("PICK");   // move the cursor within a menu
const u8 gText_lu_UI_KeybindReturnToMenu[]  = _("RETURN TO MENU"); // return from an informational screen
const u8 gText_lu_UI_KeybindTypeIn[]        = _("TYPE IN");
const u8 gText_lu_UI_KeybindViewSearchResults[] = _("VIEW SEARCH RESULTS");

const u8 gText_lu_CGO_menuTitle[] = _("CUSTOM GAME OPTIONS");

const u8 gText_lu_CGOptionName_StartWithRunningShoes[] = _("Start w/ running");
const u8 gText_lu_CGOptionHelp_StartWithRunningShoes[] = _("In the original Pokémon Emerald, you could only run on the overworld after receiving your first Pokémon, your Pokédex, and the Running Shoes. This option can be used to enable running from the moment you start the game.");

const u8 gText_lu_CGOptionName_AllowRunningIndoors[] = _("Allow running indoors");

const u8 gText_lu_CGOptionName_AllowBikingIndoors[] = _("Allow biking indoors");

const u8 gText_lu_CGOptionName_InfiniteUseTMs[] = _("Infinite-use TMs");
const u8 gText_lu_CGOptionHelp_InfiniteUseTMs[] = _("In the original Pokémon Emerald, TMs can only be used once, and are destroyed afterward. In newer games, TMs can be used an unlimited number of times. This option can enable the latter behavior.");


//

const u8 gText_lu_CGOptionCategoryName_StarterPokemon[] = _("Starter Pokémon");
const u8 gText_lu_CGOptionCategoryHelp_StarterPokemon[] = _("Modify the three starter Pokémon that the player can choose from at the beginning of the game.");
//
const u8 gText_lu_CGOptionName_Starters_Species_0[] = _("Starter Species 1");
const u8 gText_lu_CGOptionHelp_Starters_Species_0[] = _("Override the lefthand starter option.\n\nDefault: Treecko");
const u8 gText_lu_CGOptionName_Starters_Species_1[] = _("Starter Species 2");
const u8 gText_lu_CGOptionHelp_Starters_Species_1[] = _("Override the middle starter option.\n\nDefault: Torchic");
const u8 gText_lu_CGOptionName_Starters_Species_2[] = _("Starter Species 3");
const u8 gText_lu_CGOptionHelp_Starters_Species_2[] = _("Override the righthand starter option.\n\nDefault: Mudkip");
const u8 gText_lu_CGOptionName_Starters_Gender[] = _("Player starter gender");
const u8 gText_lu_CGOptionHelp_Starters_Gender[] = _("Change the gender of the player's starter. This option will do nothing if the starter species chosen by the player is always gender-unknown or always a particular gender.\n\nDefault: Random");
const u8 gText_lu_CGOptionName_Starters_Level[] = _("Player starter level");
const u8 gText_lu_CGOptionHelp_Starters_Level[] = _("Change the level of the player's starter.\n\nDefault: 5");
//
const u8 gText_lu_CGOptionValueName_Starters_Gender_Male[] = _("Male");
const u8 gText_lu_CGOptionValueName_Starters_Gender_Female[] = _("Female");

const u8 gText_lu_CGOptionValues_common_Always[] = _("Always");
const u8 gText_lu_CGOptionValues_common_Default[] = _("Default");
const u8 gText_lu_CGOptionValues_common_Disabled[] = _("Disabled");
const u8 gText_lu_CGOptionValues_common_Enabled[]  = _("Enabled");
const u8 gText_lu_CGOptionValues_common_Never[]  = _("Never");
const u8 gText_lu_CGOptionValues_common_None[]  = _("None");
const u8 gText_lu_CGOptionValues_common_Random[]  = _("Random");