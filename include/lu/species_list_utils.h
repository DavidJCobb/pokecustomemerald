#ifndef GUARD_LU_SPECIES_LIST_UTILS_H
#define GUARD_LU_SPECIES_LIST_UTILS_H

#include "lu/game_typedefs.h"

struct SpeciesList {
   u16 count;
   PokemonSpeciesID* speciesIDs;
};

typedef bool8(*SpeciesListComparatorLess)(PokemonSpeciesID, PokemonSpeciesID);

// Returns NULL if the list would be empty.
extern void AllocFilteredSpeciesList(struct SpeciesList*, bool8(*filter)(PokemonSpeciesID));

extern void SortSpeciesList(struct SpeciesList*, SpeciesListComparatorLess);

// Creates a task and returns a task ID. If the list can be sorted instantly 
// (i.e. if it has two or fewer items), this invokes the completion callback 
// immediately and returns TASK_NONE.
extern u8 SortSpeciesListAsync(
   struct SpeciesList*,
   SpeciesListComparatorLess,
   u16 maxComparisonsPerRun,
   void(*onCompleteCallback)(void)
);

#endif