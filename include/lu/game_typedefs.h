#ifndef GUARD_LU_TEST_GAME_TYPEDEFS
#define GUARD_LU_TEST_GAME_TYPEDEFS

#include "lu/bitpack_options.h"
#include "gba/types.h"

#include "constants/global.h"
// - NUM_LANGUAGES
// - PLAYER_NAME_LENGTH
// - POKEMON_NAME_LENGTH
// - TRAINER_ID_LENGTH
#include "constants/pokemon.h"
// - MAX_LEVEL

//
// Typedefs, to make some things (e.g. bitpacking options and stats) easier.
//

// Level modes for the Battle Frontier facilities and for 
// the Battle Tent.
LU_BP_MINMAX(0, FRONTIER_LVL_TENT) typedef u8 BattleFacilityLvlMode;

// Level modes for the Battle Frontier faciltiies, not 
// including the Battle Tent.
LU_BP_MINMAX(0, FRONTIER_LVL_MODE_COUNT - 1) typedef u8 BattleFrontierLvlMode;

// Level modes for the Battle Frontier faciltiies, not 
// including the Battle Tent. Values are shifted by 1 
// such that 0 indicates "none."
LU_BP_MINMAX(0, FRONTIER_LVL_TENT) typedef u8 BattleFrontierLvlModeOptional;

LU_BP_MINMAX(0, CONTEST_CATEGORIES_COUNT - 1) typedef u8 ContestCategory;
LU_BP_CATEGORY("easy-chat-word") typedef u16 EasyChatWordID;
LU_BP_CATEGORY("language") LU_BP_BITCOUNT(4) LU_BP_MINMAX(0, NUM_LANGUAGES) typedef u8 LanguageID;
LU_BP_CATEGORY("global-item-id") typedef u16 ItemIDGlobal;

// Typedefs for map groups, map nums, and map sections.
typedef s8 MapGroupIndexOptional;
typedef u8 MapGroupIndex;
//
typedef s8 MapIndexInGroupOptional;
typedef u8 MapIndexInGroup;
//
typedef u8 MapSectionID; // MAPSEC_...
typedef MapSectionID MetLocation; // MAPSEC_... or METLOC_...

// See LevelUpMove in pokemon.h
LU_BP_CATEGORY("move-id") LU_BP_BITCOUNT(9) typedef u16 MoveID;

LU_BP_CATEGORY("player-gender") LU_BP_MINMAX(0, GENDER_COUNT - 1) typedef u8 PlayerGender;
LU_BP_CATEGORY("player-name")  LU_BP_STRING_WT typedef u8 PlayerName[PLAYER_NAME_LENGTH + 1];
LU_BP_CATEGORY("player-name")  LU_BP_STRING_UT typedef u8 PlayerNameNoTerminator[PLAYER_NAME_LENGTH];
LU_BP_CATEGORY("pokemon-level") LU_BP_MINMAX(0,MAX_LEVEL) typedef u8 PokemonLevel;
LU_BP_CATEGORY("pokemon-name") LU_BP_STRING_WT typedef u8 PokemonName[POKEMON_NAME_LENGTH + 1];
LU_BP_CATEGORY("pokemon-name") LU_BP_STRING_UT typedef u8 PokemonNameNoTerminator[POKEMON_NAME_LENGTH];
typedef u32 PokemonPersonality;
LU_BP_CATEGORY("species-id") LU_BP_MINMAX(0,2047) typedef u16 PokemonSpeciesID;

// NOTE: Trainer IDs have some other representations as well.
LU_BP_CATEGORY("trainer-id") typedef u8 TrainerID[TRAINER_ID_LENGTH];

#endif