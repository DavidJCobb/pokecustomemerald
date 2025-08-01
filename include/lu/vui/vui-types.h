#ifndef GUARD_LU_V_WIDGETS_TYPES_H
#define GUARD_LU_V_WIDGETS_TYPES_H

#include "lu/c-attr.define.h"
#include "gba/defines.h" // TRUE, FALSE; stdddef.h: NULL and friends
#include "gba/types.h"

enum VUIAxis {
   VUI_AXIS_X = 0,
   VUI_AXIS_Y = 1,
};

enum VUIDirection {
   VUI_DIRECTION_NONE  = -1,
   VUI_DIRECTION_LEFT  =  0,
   VUI_DIRECTION_RIGHT =  1,
   VUI_DIRECTION_UP    =  2,
   VUI_DIRECTION_DOWN  =  3,
};
#define VUI_DirectionToAxis(d) (enum VUIAxis)((u8)(d) / 2)
inline enum VUIDirection VUI_DeltaToDirection(s8 dx, s8 dy) {
   if (dx)
      return (dx < 0) ? VUI_DIRECTION_LEFT : VUI_DIRECTION_RIGHT;
   if (dy)
      return (dy < 0) ? VUI_DIRECTION_UP : VUI_DIRECTION_DOWN;
   return VUI_DIRECTION_NONE;
}

typedef struct VUIExtent {
   u8 start;
   u8 end;
} VUIExtent;

NON_NULL_PARAMS(1)   extern u8 VUI_ExtentDistance(const VUIExtent*, u8);
NON_NULL_PARAMS(1,2) extern bool8 VUI_ExtentsOverlap(const VUIExtent*, const VUIExtent*);
NON_NULL_PARAMS(1)   extern bool8 VUI_ExtentOverlaps(const VUIExtent*, u8);
NON_NULL_PARAMS(1)   extern s16 VUI_PointBeyondExtent(const VUIExtent*, s8);

typedef union VUIPos {
   u8 coords[2];
   struct {
      u8 x;
      u8 y;
   };
} VUIPos;

typedef union VUISize {
   u8 bounds[2];
   struct {
      u8 w;
      u8 h;
   };
} VUISize;

NON_NULL_PARAMS(1,2)     extern void VUI_ConstrainPos(VUIPos*, const VUISize*);
NON_NULL_PARAMS(1,2,3,4) extern void VUI_MapBoxToExtents(const VUIPos*, const VUISize*, VUIExtent* x, VUIExtent* y);
NON_NULL_PARAMS(1,2,3)   extern void VUI_MapPosAcrossSizes(VUIPos*, const VUISize* src, const VUISize* dst);

typedef struct VUIGridArea {
   VUIPos  pos;
   VUISize size;
} VUIGridArea;

typedef union VUITextColors {
   u8 list[3];
   struct {
      u8 back;
      u8 text;
      u8 shadow;
   };
} VUITextColors;

typedef struct VUIStringRef {
   u8* data;
   u8  size; // capacity, not including space for a terminating EOS
} VUIStringRef;

// Constructor. The `this` parameter must be a VUIStringRef*, and 
// `buffer` must be a byte array (not a pointer, even one which 
// originally decayed from an array).
#define VUIStringRef_Construct(this, buffer) \
   _Static_assert(                           \
      1 == _Generic(                         \
         (buffer),                           \
         u8(*)[sizeof(buffer)] : 1,          \
         default:                0           \
      ),                                     \
      "VUIStringRef_Construct must be used " \
      "on a char array, not a char pointer"  \
   );                                        \
   (this)->data = (buffer);                  \
   (this)->size = sizeof(buffer) - 1;

NON_NULL_PARAMS(1) extern void  VUIStringRef_Clear(VUIStringRef*);
NON_NULL_PARAMS(1) extern bool8 VUIStringRef_IsNull(const VUIStringRef*);

#include "lu/c-attr.undef.h"
#endif