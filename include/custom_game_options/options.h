#ifndef GUARD_LU_CUSTOM_GAME_OPTIONS
#define GUARD_LU_CUSTOM_GAME_OPTIONS

#include "custom_game_options/options_set.h"
#include "gba/defines.h"

// Track current values of Custom Game options. Intended to be serialized after SaveBlock2.
extern struct CustomGameOptionsSet gCustomGameOptions;

// Track in-game progress related to custom game options. Intended to be serialized 
// after SaveBlock2.
//
// Needed for Nuzlocke encounter options: we should not enforce any encounter/catch 
// limits until the player has obtained at least one of any Poke Ball type.
extern struct CustomGameSavestate {
} gCustomGameSavestate;

extern void ResetCustomGameOptions(void);
extern void ResetCustomGameSavestate(void);

// Set the values of available custom game options to mimic the game mechanics of a 
// given generation of Pokemon games.
extern void SetCustomGameOptionsPerGeneration(struct CustomGameOptionsSet*, u8 generation);

extern void CustomGames_HandleNewPlaythroughStarted(void);

#endif