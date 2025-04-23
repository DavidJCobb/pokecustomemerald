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

#define APPLY_COMB_SORT_SHRINK_FACTOR(x) (x / 4 * 3)

static bool8 TryInstantSort(
   struct SpeciesList* species_list,
   bool8(*comparatorLess)(PokemonSpeciesID, PokemonSpeciesID)
) {
   if (species_list->count < 2) {
      return TRUE;
   } else if (species_list->count == 2) {
      PokemonSpeciesID a = species_list->speciesIDs[0];
      PokemonSpeciesID b = species_list->speciesIDs[1];
      if (!comparatorLess(a, b)) {
         species_list->speciesIDs[0] = b;
         species_list->speciesIDs[1] = a;
      }
      return TRUE;
   }
   return FALSE;
}

void SortSpeciesList(
   struct SpeciesList* species_list,
   bool8(*comparatorLess)(PokemonSpeciesID, PokemonSpeciesID)
) {
   if (TryInstantSort(species_list, comparatorLess))
      return;
   //
   // Comb sort.
   //
   u32   gap    = species_list->count;
   bool8 sorted = FALSE;
   while (!sorted) {
      gap = APPLY_COMB_SORT_SHRINK_FACTOR(gap);
      if (gap <= 1) {
         gap    = 1;
         sorted = TRUE; // if we don't swap anything this time, we're done
      } else if (gap == 9 || gap == 10) {
         gap = 11;
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

#include "task.h"
#include "gba/isagbprint.h"

#define tMaxOps       data[0]
#define tComparatorA  data[1]
#define tComparatorB  data[2]
#define tCallbackA    data[3]
#define tCallbackB    data[4]
#define tSpeciesListA data[5]
#define tSpeciesListB data[6]
#define tGap          data[7]
#define tIndex        data[8]
#define tSorted       data[9]

typedef void(*AsyncSortCompletionCallback)(void);

void Task_SortSpeciesListAsyncDone(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   
   AsyncSortCompletionCallback callback = (AsyncSortCompletionCallback) GetWordTaskArg(taskId, 3);
   DestroyTask(taskId);
   (callback)();
}

void Task_SortSpeciesListAsync(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   
   SpeciesListComparatorLess comparator_less = (SpeciesListComparatorLess) GetWordTaskArg(taskId, 1);
   struct SpeciesList* species_list = (struct SpeciesList*) GetWordTaskArg(taskId, 5);
   
   u16       ops     = 0;
   const u16 max_ops = (u16)task->tMaxOps;
   u16       gap     = (u16)task->tGap;
   bool8     sorted  = FALSE;
   if (task->tIndex > 0) {
      sorted = task->tSorted;
      //
      // Finish an in-progress pass.
      //
      for(u16 i = task->tIndex; i + gap < species_list->count; ++i) {
         PokemonSpeciesID x = species_list->speciesIDs[i];
         PokemonSpeciesID y = species_list->speciesIDs[i + gap];
         if (!comparator_less(x, y)) {
            species_list->speciesIDs[i]       = y;
            species_list->speciesIDs[i + gap] = x;
            sorted = FALSE;
         }
         if (++ops >= max_ops) {
            task->tGap    = gap;
            task->tIndex  = i + 1;
            task->tSorted = sorted;
            return;
         }
      }
      if (sorted) {
         Task_SortSpeciesListAsyncDone(taskId);
         return;
      }
   }
   //
   // Handle whole passes.
   //
   while (!sorted) {
      gap = APPLY_COMB_SORT_SHRINK_FACTOR(gap);
      if (gap == 9 || gap == 10)
         gap = 11;
      else if (gap <= 1) {
         gap    = 1;
         sorted = TRUE; // if we don't swap anything this time, we're done
      }
      
      for(u16 i = 0; i + gap < species_list->count; ++i) {
         PokemonSpeciesID x = species_list->speciesIDs[i];
         PokemonSpeciesID y = species_list->speciesIDs[i + gap];
         if (!comparator_less(x, y)) {
            species_list->speciesIDs[i]       = y;
            species_list->speciesIDs[i + gap] = x;
            sorted = FALSE;
         }
         if (++ops >= max_ops) {
            task->tGap    = gap;
            task->tIndex  = i + 1;
            task->tSorted = sorted;
            return;
         }
      }
   }
   
   Task_SortSpeciesListAsyncDone(taskId);
}

extern u8 SortSpeciesListAsync(
   struct SpeciesList* species_list,
   bool8(*comparatorLess)(PokemonSpeciesID, PokemonSpeciesID),
   u16 max_comparisons_per_run,
   void(*on_complete_callback)(void)
) {
   if (TryInstantSort(species_list, comparatorLess)) {
      on_complete_callback();
      return TASK_NONE;
   }
   u8 taskId = CreateTask(Task_SortSpeciesListAsync, 50);
   
   struct Task* task = &gTasks[taskId];
   
   task->tMaxOps = max_comparisons_per_run;
   SetWordTaskArg(taskId, 1, (u32)comparatorLess);
   SetWordTaskArg(taskId, 3, (u32)on_complete_callback);
   SetWordTaskArg(taskId, 5, (u32)species_list);
   {
      task->tGap = species_list->count;
      if (task->tGap == 9 || task->tGap == 10)
         task->tGap = 11;
   }
   task->tIndex  = 0;
   task->tSorted = FALSE;
   
   return taskId;
}

#undef tMaxOps
#undef tComparatorA
#undef tComparatorB
#undef tCallbackA
#undef tCallbackB
#undef tSpeciesListA
#undef tSpeciesListB
#undef tGap
#undef tIndex