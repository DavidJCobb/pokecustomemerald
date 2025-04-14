#ifndef GUARD_LU_GENERATE_POKEMON_H
#define GUARD_LU_GENERATE_POKEMON_H

#include "global.h"
#include "lu/game_typedefs.h"

enum { // flag indices
   GENERATE_POKEMON_WITH_GENDER,
   GENERATE_POKEMON_WITH_NATURE,
   GENERATE_POKEMON_WITH_PERSONALITY,
   GENERATE_POKEMON_WITH_OT_ID,
   GENERATE_POKEMON_WITH_OT_ID_OF_PLAYER,
   GENERATE_POKEMON_WITH_SHININESS_SET,
   GENERATE_POKEMON_WITH_UNOWN_LETTER,
};

struct GeneratePokemonRequest {
   u8 flags;
   
   u8               fixed_iv; // or USE_RANDOM_IVS
   u8               gender;
   ItemIDGlobal     held_item;
   PokemonLevel     level;
   u8               nature;
   u32              ot_id;
   u32              personality;
   bool8            shiny : 1;
   PokemonSpeciesID species;
   u8               unown_letter;
};

extern void GeneratePokemon(struct Pokemon* dst, const struct GeneratePokemonRequest*);

#endif