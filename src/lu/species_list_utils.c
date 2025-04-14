#include "lu/species_list_utils.h"
#include "constants/species.h"
#include "global.h"
#include "malloc.h"

void AllocFilteredSpeciesList(
   struct SpeciesList* species_list,
   bool8(*filter)(PokemonSpeciesID)
) {
   if (species_list->speciesIDs) {
      Free(species_list->speciesIDs);
      species_list->speciesIDs = NULL;
   }
   species_list->count = 0;
   
   u8*              flags = (u8*)AllocZeroed(NUM_SPECIES / 8 + (NUM_SPECIES % 8 ? 1 : 0));
   bool8            any   = FALSE;
   PokemonSpeciesID first = 0;
   PokemonSpeciesID count = 0;
   for(int i = 0; i < NUM_SPECIES; ++i) {
      if (filter(i)) {
         flags[i / 8] |= 1 << (i % 8);
         any = TRUE;
         ++count;
         if (first == 0)
            first = i;
      }
   }
   if (any) {
      species_list->count      = count;
      species_list->speciesIDs = (PokemonSpeciesID*)AllocZeroed(count * sizeof(PokemonSpeciesID));
      
      int dst = 0;
      for(int i = first; i < NUM_SPECIES; ++i) {
         bool8 eligible = (flags[i / 8] & (1 << (i % 8))) != 0;
         if (eligible) {
            species_list->speciesIDs[dst] = i;
            ++dst;
            if (dst >= count)
               break;
         }
      }
   }
   Free(flags);
}

//

#define APPLY_SHRINK_SORT_SHRINK_FACTOR(x) (x / 4 * 3)

void SortSpeciesList(
   struct SpeciesList* species_list,
   bool8(*comparatorLess)(PokemonSpeciesID, PokemonSpeciesID)
) {
   if (species_list->count < 2) {
      return;
   } else if (species_list->count == 2) {
      PokemonSpeciesID a = species_list->speciesIDs[0];
      PokemonSpeciesID b = species_list->speciesIDs[1];
      if (!comparatorLess(a, b)) {
         species_list->speciesIDs[0] = b;
         species_list->speciesIDs[1] = a;
      }
      return;
   }
   //
   // Comb sort.
   //
   u32   gap    = species_list->count;
   bool8 sorted = FALSE;
   if (gap >= 9) {
      gap = 11;
   }
   while (!sorted) {
      gap = APPLY_SHRINK_SORT_SHRINK_FACTOR(gap);
      if (gap <= 1) {
         gap    = 1;
         sorted = TRUE; // if we don't swap anything this time, we're done
      }
      
      for(int i = 0; i + gap < species_list->count; ++i) {
         PokemonSpeciesID x = species_list->speciesIDs[i];
         PokemonSpeciesID y = species_list->speciesIDs[i + gap];
         if (!comparatorLess(x, y)) {
            species_list->speciesIDs[i]       = y;
            species_list->speciesIDs[i + gap] = x;
            sorted = FALSE;
         }
      }
   }
}