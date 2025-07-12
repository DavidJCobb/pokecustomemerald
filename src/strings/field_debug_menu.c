#include "strings/field_debug_menu.h"

const u8 gText_lu_FieldDebugMenu_DisableTrainerLOS[] = _("Disable trainer LOS");
const u8 gText_lu_FieldDebugMenu_DisableWildEncounters[] = _("Disable wild encounters");
const u8 gText_lu_FieldDebugMenu_FastTravel[] = _("Fast-travel");
const u8 gText_lu_FieldDebugMenu_UseAnyBike[] = _("Use any bike");
const u8 gText_lu_FieldDebugMenu_UseAnyFieldMove[] = _("Use any field move");
const u8 gText_lu_FieldDebugMenu_UseAnyFishingRod[] = _("Use any fishing rod");
const u8 gText_lu_FieldDebugMenu_SetTime[] = _("Set time of day");
const u8 gText_lu_FieldDebugMenu_SetWeather[] = _("Set weather");
const u8 gText_lu_FieldDebugMenu_TestBattleTransition[] = _("Test battle transition");
const u8 gText_lu_FieldDebugMenu_WalkThroughWalls[] = _("Walk through walls");

const u8 gText_lu_FieldDebugMenu_UseAnyBike_Acro[] = _("Acro Bike");
const u8 gText_lu_FieldDebugMenu_UseAnyBike_Mach[] = _("Mach Bike");

const u8 gText_lu_FieldDebugMenu_UseAnyFishingRod_Old[] = _("Old Rod");
const u8 gText_lu_FieldDebugMenu_UseAnyFishingRod_Good[] = _("Good Rod");
const u8 gText_lu_FieldDebugMenu_UseAnyFishingRod_Super[] = _("Super Rod");

const u8 gText_lu_FieldDebugMenu_SetWeather_SunnyCloudy[] = _("Sunny + clouds");
const u8 gText_lu_FieldDebugMenu_SetWeather_Sunny[] = _("Sunny");
const u8 gText_lu_FieldDebugMenu_SetWeather_Rain[] = _("Rain");
const u8 gText_lu_FieldDebugMenu_SetWeather_Snow[] = _("Snow");
const u8 gText_lu_FieldDebugMenu_SetWeather_Thunderstorm[] = _("Thunderstorm");
const u8 gText_lu_FieldDebugMenu_SetWeather_Fog[] = _("Fog");
const u8 gText_lu_FieldDebugMenu_SetWeather_Ashfall[] = _("Ashfall");
const u8 gText_lu_FieldDebugMenu_SetWeather_Sandstorm[] = _("Sandstorm");
const u8 gText_lu_FieldDebugMenu_SetWeather_FogDiagonal[] = _("Fog Diagonal");
const u8 gText_lu_FieldDebugMenu_SetWeather_Underwater[] = _("RS Underwater");
const u8 gText_lu_FieldDebugMenu_SetWeather_Overcast[] = _("Overcast");
const u8 gText_lu_FieldDebugMenu_SetWeather_Drought[] = _("RS Drought");
const u8 gText_lu_FieldDebugMenu_SetWeather_Downpour[] = _("RS Downpour");
const u8 gText_lu_FieldDebugMenu_SetWeather_UnderwaterBubbles[] = _("Underwater");
const u8 gText_lu_FieldDebugMenu_SetWeather_Abnormal[] = _("Abnormal");
const u8 gText_lu_FieldDebugMenu_SetWeather_Route119Cycle[] = _("Route 119");
const u8 gText_lu_FieldDebugMenu_SetWeather_Route123Cycle[] = _("Route 123");

#define BATTLE_TRANSITION_MENU_ENTRY_DECL(name) \
   const u8 gText_lu_FieldDebugMenu_BattleTransition_##name []
#define BATTLE_TRANSITION_MENU_ENTRY_STRING(name) \
   BATTLE_TRANSITION_MENU_ENTRY_DECL(name) = _(#name);
BATTLE_TRANSITION_MENU_ENTRY_DECL(BLUR) = _("Blur");
BATTLE_TRANSITION_MENU_ENTRY_DECL(SWIRL) = _("Swirl");
BATTLE_TRANSITION_MENU_ENTRY_DECL(SHUFFLE) = _("Shuffle");
BATTLE_TRANSITION_MENU_ENTRY_DECL(BIG_POKEBALL) = _("Big Poke Ball");
BATTLE_TRANSITION_MENU_ENTRY_DECL(POKEBALLS_TRAIL) = _("Poke Balls Trail");
BATTLE_TRANSITION_MENU_ENTRY_DECL(CLOCKWISE_WIPE) = _("Clockwise Wipe");
BATTLE_TRANSITION_MENU_ENTRY_DECL(RIPPLE) = _("Ripple");
BATTLE_TRANSITION_MENU_ENTRY_DECL(WAVE) = _("Wave");
BATTLE_TRANSITION_MENU_ENTRY_DECL(SLICE) = _("Slice");
BATTLE_TRANSITION_MENU_ENTRY_DECL(WHITE_BARS_FADE) = _("White Bars Fade");
BATTLE_TRANSITION_MENU_ENTRY_DECL(GRID_SQUARES) = _("Grid Squares");
BATTLE_TRANSITION_MENU_ENTRY_DECL(ANGLED_WIPES) = _("Angled Wipes");
BATTLE_TRANSITION_MENU_ENTRY_DECL(SIDNEY) = _("E4 Sidney");
BATTLE_TRANSITION_MENU_ENTRY_DECL(PHOEBE) = _("E4 Phoebe");
BATTLE_TRANSITION_MENU_ENTRY_DECL(GLACIA) = _("E4 Glacia");
BATTLE_TRANSITION_MENU_ENTRY_DECL(DRAKE) = _("E4 Drake");
BATTLE_TRANSITION_MENU_ENTRY_DECL(CHAMPION) = _("Champion");
BATTLE_TRANSITION_MENU_ENTRY_DECL(AQUA) = _("Team Aqua");
BATTLE_TRANSITION_MENU_ENTRY_DECL(MAGMA) = _("Team Magma");
BATTLE_TRANSITION_MENU_ENTRY_STRING(REGICE)
BATTLE_TRANSITION_MENU_ENTRY_STRING(REGISTEEL)
BATTLE_TRANSITION_MENU_ENTRY_STRING(REGIROCK)
BATTLE_TRANSITION_MENU_ENTRY_STRING(KYOGRE)
BATTLE_TRANSITION_MENU_ENTRY_STRING(GROUDON)
BATTLE_TRANSITION_MENU_ENTRY_STRING(RAYQUAZA)
BATTLE_TRANSITION_MENU_ENTRY_DECL(SHRED_SPLIT) = _("Shred Split");
BATTLE_TRANSITION_MENU_ENTRY_DECL(BLACKHOLE) = _("Black Hole");
BATTLE_TRANSITION_MENU_ENTRY_DECL(BLACKHOLE_PULSATE) = _("Black Hole Pulsate");
BATTLE_TRANSITION_MENU_ENTRY_DECL(RECTANGULAR_SPIRAL) = _("Rect. Spiral");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_LOGO_WIGGLE) = _("Frontier Logo Wiggle");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_LOGO_WAVE) = _("Frontier Logo Wave");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_SQUARES) = _("Frontier Sq.");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_SQUARES_SCROLL) = _("Fntr. Sq. Scroll");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_SQUARES_SPIRAL) = _("Fntr. Sq. Spiral");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_CIRCLES_MEET) = _("Fntr. Circles Meet");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_CIRCLES_CROSS) = _("Fntr. Circles Cross");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_CIRCLES_ASYMMETRIC_SPIRAL) = _("Fntr. Spiral Asym.");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_CIRCLES_SYMMETRIC_SPIRAL) = _("Fntr. Spiral Sym.");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_CIRCLES_MEET_IN_SEQ) = _("Fntr. Circ. Meet Seq.");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_CIRCLES_CROSS_IN_SEQ) = _("Fntr. Circ. Cross Seq.");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_CIRCLES_ASYMMETRIC_SPIRAL_IN_SEQ) = _("Fntr. Spi. Asym. Seq.");
BATTLE_TRANSITION_MENU_ENTRY_DECL(FRONTIER_CIRCLES_SYMMETRIC_SPIRAL_IN_SEQ) = _("Fntr. Spi. Sym. Seq.");
#undef BATTLE_TRANSITION_MENU_ENTRY_DECL
#undef BATTLE_TRANSITION_MENU_ENTRY_STRING