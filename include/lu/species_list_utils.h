#ifndef GUARD_LU_SPECIES_LIST_UTILS_H
#define GUARD_LU_SPECIES_LIST_UTILS_H

#include "lu/game_typedefs.h"

struct SpeciesList {
   u16 count;
   PokemonSpeciesID* speciesIDs;
};

// Returns NULL if the list would be empty.
extern void AllocFilteredSpeciesList(struct SpeciesList*, bool8(*filter)(PokemonSpeciesID));

extern void SortSpeciesList(struct SpeciesList*, bool8(*comparatorLess)(PokemonSpeciesID, PokemonSpeciesID));

#endif