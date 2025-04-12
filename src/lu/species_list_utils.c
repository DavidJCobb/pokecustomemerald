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

#define XOR_SWAP(x, y) do { x = y ^ x; y = x ^ y; x = y ^ x; } while (0)

static u16 PartitionSpeciesList(
   struct SpeciesList* species_list,
   u16 lo,
   u16 hi
) {
   PokemonSpeciesID pivot = species_list->speciesIDs[hi];
   
   u16 i = lo;
   for(u16 j = lo; j < hi; ++j) {
      if (species_list->speciesIDs[j] <= pivot) {
         XOR_SWAP(
            species_list->speciesIDs[i],
            species_list->speciesIDs[j]
         );
         ++i;
      }
   }
   XOR_SWAP(
      species_list->speciesIDs[i],
      species_list->speciesIDs[hi]
   );
   return i;
}
static void QuicksortSpeciesList(
   struct SpeciesList* species_list,
   u16 lo,
   u16 hi
) {
   if (lo >= hi) {
      return;
   }
   u16 pivot = PartitionSpeciesList(species_list, lo, hi);
   
   QuicksortSpeciesList(species_list, lo, pivot - 1);
   QuicksortSpeciesList(species_list, pivot + 1, hi);
}

void SortSpeciesList(
   struct SpeciesList* species_list,
   bool8(*comparatorLess)(PokemonSpeciesID, PokemonSpeciesID)
) {
   if (species_list->count < 2) {
      return;
   } else if (species_list->count == 2) {
      PokemonSpeciesID a = species_list->speciesIDs[0];
      PokemonSpeciesID b = species_list->speciesIDs[1];
      if (a > b) {
         XOR_SWAP(
            species_list->speciesIDs[0],
            species_list->speciesIDs[1]
         );
      }
      return;
   }
   QuicksortSpeciesList(
      species_list,
      0,
      species_list->count - 1
   );
}