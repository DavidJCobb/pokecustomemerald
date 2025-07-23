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
extern s16 VUI_PointBeyondExtent(const VUIExtent* extent, s8 d) {
   if (d < 0) {
      return extent->start + d;
   } else {
      return extent->end - 1 + d;
   }
}

extern void VUI_ConstrainPos(VUIPos* pos, const VUISize* size) {
   if (pos->x >= size->w)
      pos->x = size->w - 1;
   if (pos->y >= size->h)
      pos->y = size->h - 1;
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

#include "constants/characters.h"

extern void VUIStringRef_Clear(VUIStringRef* v) {
   if (!v->data || !v->size)
      return;
   for(u8 i = 0; i < v->size; ++i)
      v->data[i] = EOS;
}
extern bool8 VUIStringRef_IsNull(const VUIStringRef* v) {
   return !v->data || !v->size;
}