#include "lu/vui/vui-types.h"

extern u8 VUI_ExtentDistance(const VUIExtent* extent, u8 point) {
   if (extent->start <= point) {
      if (extent->end > point)
         return 0;
      return point - extent->end + 1;
   }
   return extent->start - point;
}
extern bool8 VUI_ExtentsOverlap(const VUIExtent* a, const VUIExtent* b) {
   if (a->end <= b->start)
      return FALSE;
   if (a->start >= b->end)
      return FALSE;
   return TRUE;
}
extern bool8 VUI_ExtentOverlaps(const VUIExtent* extent, u8 point) {
   if (extent->end <= point)
      return FALSE;
   if (extent->start > point)
      return FALSE;
   return TRUE;
}

extern void VUI_MapBoxToExtents(
   const VUIPos*    pos,
   const VUISize*   size,
   VUIExtent* x,
   VUIExtent* y
) {
   x->start = pos->x;
   y->start = pos->y;
   x->end   = pos->x + size->w;
   y->end   = pos->y + size->h;
}
extern void VUI_MapPosAcrossSizes(VUIPos* pos, const VUISize* src, const VUISize* dst) {
   if (dst->w) {
      if (src->w && src->w != dst->w)
         pos->x = (pos->x * 10 * dst->w) / src->w / 10;
      if (pos->x >= dst->w)
         pos->x = dst->w - 1;
   }
   if (dst->h) {
      if (src->h && src->h != dst->h)
         pos->y = (pos->y * 10 * dst->h) / src->h / 10;
      if (pos->y >= dst->h)
         pos->y = dst->h - 1;
   }
}