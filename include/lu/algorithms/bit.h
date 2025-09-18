#ifndef GUARD_LU_BIT
#define GUARD_LU_BIT

#include "gba/types.h"
#ifndef CHAR_BIT
   #include <limits.h>
#endif

inline static u8    BitCeil8(u8);
inline static u16   BitCeil16(u16);
inline static u8    BitCountRZero8(u8);   // Count number of zeroed least-significant bits
inline static u8    BitCountRZero16(u16); //
inline static u8    BitFloor8(u8);
inline static u16   BitFloor16(u16);
inline static u8    BitWidth8(u8);
inline static u8    BitWidth16(u16);
inline static bool8 HasSingleBit8(u8);
inline static bool8 HasSingleBit16(u16);
inline static u8    Popcount8(u8);
inline static u8    Popcount16(u16);

//
// Definitions below.
//

inline static u8 BitCeil8(u8 v) {
   if (v <= 1)
      return 1;
   --v;
   v |= v >> 1;
   v |= v >> 2;
   v |= v >> 4;
   ++v;
   return v;
   /*//
   return (u8)1 << BitWidth8(v - 1);
   //*/
}
inline static u16 BitCeil16(u16 v) {
   if (v <= 1)
      return 1;
   --v;
   v |= v >> 1;
   v |= v >> 2;
   v |= v >> 4;
   v |= v >> 8;
   ++v;
   return v;
   /*//
   return (u16)1 << BitWidth16(v - 1);
   //*/
}

#define DEFINE_FUNC(Name, T)          \
   inline static u8 Name(T v) {       \
      if (!v)                         \
         return CHAR_BIT * sizeof(v); \
      v = (v ^ (v - 1)) >> 1; /* set trailing 0s to 1s, and clear the rest */ \
      u8 count = 0;                   \
      for(; v; ++count)               \
         v >>= 1;                     \
      return count;                   \
   }
DEFINE_FUNC(BitCountRZero8,  u8);
DEFINE_FUNC(BitCountRZero16, u16);
#undef DEFINE_FUNC

inline static u8 BitFloor8(u8 v) {
   if (v)
      return (u8)1 << (BitWidth8(v) - 1);
   return 0;
}
inline static u16 BitFloor16(u16 v) {
   if (v)
      return (u16)1 << (BitWidth16(v) - 1);
   return 0;
}

#define DEFINE_FUNC(Name, T) \
   inline static u8 Name(T v) {   \
      u8 width = 0;               \
      while (v >>= 1)             \
         ++width;                 \
   }
DEFINE_FUNC(BitWidth8,  u8)
DEFINE_FUNC(BitWidth16, u16)
#undef DEFINE_FUNC

#define DEFINE_FUNC(Name, T) \
   inline static bool8 Name(T v) { return v && !(v & (v - 1)); }
DEFINE_FUNC(HasSingleBit8,  u8)
DEFINE_FUNC(HasSingleBit16, u16)
#undef DEFINE_FUNC

#define DEFINE_FUNC(Name, T) \
   inline static u8 Name(T v) {  \
      u8 count = 0;              \
      for(; v; ++count)          \
         v &= v - 1;             \
      return count;              \
   }
DEFINE_FUNC(Popcount8,  u8)
DEFINE_FUNC(Popcount16, u16)
#undef DEFINE_FUNC

#endif