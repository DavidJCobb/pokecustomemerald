#ifndef GUARD_ALGO_INSERTIONSORT
#define GUARD_ALGO_INSERTIONSORT

#include "gba/types.h"

#define DEFINE_TYPED_INSERTION_SORT(Name, T)     \
   inline static void Name (T* array, const u16 size) { \
      for(s16 i = 1; i < size; ++i) {            \
         T   x = array[i];                       \
         s16 j = i;                              \
         for(; j > 0 && array[j - 1] > x; --j) { \
            array[j] = array[j - 1];             \
         }                                       \
         array[j] = x;                           \
      }                                          \
   }

DEFINE_TYPED_INSERTION_SORT(InsertionSortU8,  u8)
DEFINE_TYPED_INSERTION_SORT(InsertionSortU16, u16)

#undef DEFINE_TYPED_INSERTION_SORT

#define DEFINE_TYPED_REVERSE_INSERTION_SORT(Name, T)     \
   inline static void Name (T* array, const u16 size) { \
      for(s16 i = 1; i < size; ++i) {            \
         T   x = array[i];                       \
         s16 j = i;                              \
         for(; j > 0 && array[j - 1] < x; --j) { \
            array[j] = array[j - 1];             \
         }                                       \
         array[j] = x;                           \
      }                                          \
   }

DEFINE_TYPED_REVERSE_INSERTION_SORT(InsertionSortReverseU8,  u8)
DEFINE_TYPED_REVERSE_INSERTION_SORT(InsertionSortReverseU16, u16)

#undef DEFINE_TYPED_REVERSE_INSERTION_SORT


#endif