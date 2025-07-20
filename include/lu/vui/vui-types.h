#ifndef GUARD_LU_V_WIDGETS_TYPES_H
#define GUARD_LU_V_WIDGETS_TYPES_H

#include "gba/defines.h" // TRUE, FALSE; stdddef.h: NULL and friends
#include "gba/types.h"

typedef struct VUIExtent {
   u8 start;
   u8 end;
} VUIExtent;

extern u8 VUI_ExtentDistance(const VUIExtent*, u8);
extern bool8 VUI_ExtentsOverlap(const VUIExtent*, const VUIExtent*);
extern bool8 VUI_ExtentOverlaps(const VUIExtent*, u8);

typedef struct VUIPos {
   u8 x;
   u8 y;
} VUIPos;

typedef struct VUISize {
   u8 w;
   u8 h;
} VUISize;

extern void VUI_MapBoxToExtents(const VUIPos*, const VUISize*, VUIExtent* x, VUIExtent* y);
extern void VUI_MapPosAcrossSizes(VUIPos*, const VUISize* src, const VUISize* dst);

typedef union VUITextColors {
   u8 list[3];
   struct {
      u8 back;
      u8 text;
      u8 shadow;
   };
} VUITextColors;

#endif