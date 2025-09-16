#ifndef GUARD_ALGO_BLOCKSORT
#define GUARD_ALGO_BLOCKSORT

#include "lu/algorithms/bit.h"

inline static void BlockSortU8(u8* array, u16 size) {
   void Reverse(u8* array, u16 start, u16 end) {
      u16 mid = start + (end - start) / 2;
      for(u16 i = start; i < end; ++i) {
         u8 a = array[i];
         u8 b = array[end - i - 1];
         array[end - i - 1] = a;
         array[i]           = b;
      }
   }
   void Rotate(u8* array, u16 count, u16 start, u16 end) {
      Reverse(array, start, end);
      Reverse(array, start, start + count);
      Reverse(array, start + count, end);
   }
   void Merge(
      u8* array,
      u16 a_start,
      u16 a_end,
      u16 b_start,
      u16 b_end
   ) {
      _Static_assert(0, "TODO");
   }
   
   u8  power_of_two   = BitFloor16(size);
   u8  denominator    = power_of_two / 16;
   u16 numerator_step = size % denominator;
   u16 integer_step   = size / denominator;
   
   while (integer_step < size) {
      u16 integer_part = 0;
      u16 numerator    = 0;
      while (integer_part < size) {
         u16 start = integer_part;
         
         integer_part += integer_step;
         numerator    += numerator_step;
         if (numerator >= denominator) {
            numerator -= denominator;
            integer_part++;
         }
         u16 mid = integer_part;
         
         integer_part += integer_step;
         numerator    += numerator_step;
         if (numerator >= denominator) {
            numerator -= denominator;
            integer_part++;
         }
         
         u16 end = integer_part;
         
         if (array[end - 1] < array[start]) {
            Rotate(array, mid - start, start, end);
         } else if (array[mid - 1] > array[mid])
            Merge(array, start, mid, mid, end);
      }
      integer_step   += integer_step;
      numerator_step += numerator_step;
      if (numerator_step >= denominator) {
         numerator_step -= denominator;
         ++integer_step;
      }
   }
}

#endif