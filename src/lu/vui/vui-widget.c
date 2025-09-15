#include "lu/vui/vui-widget.h"
#include <string.h> // memset

static const struct VTable_VUIWidget sNullVTable_Widget = {0};
/*extern*/ const struct VTable_VUIWidget gVTable_VUIWidget = {0};

extern void VUIWidget_Construct(VUIWidget* this) {
   memset(this, 0, sizeof(VUIWidget));
   this->functions = &sNullVTable_Widget;
   this->size      = (VUISize){ 1, 1 };
}
extern void VUIWidget_Destroy(VUIWidget* this) {
   if (!this)
      return;
   if (v_can_invoke(this, _impl_destroy))
      v_invoke(this, _impl_destroy)();
   this->functions = &sNullVTable_Widget;
   this->disabled  = TRUE;
   this->focusable = TRUE;
}
extern bool8 VUIWidget_IsFocusable(const VUIWidget* this) {
   return this && this->focusable && !this->disabled;
}

extern void VUIWidget_SetGridMetrics_(VUIWidget* this, u8 x, u8 y, u8 w, u8 h) {
   this->pos.x = x;
   this->pos.y = y;
   this->size.w = w ? w : 1;
   this->size.h = h ? h : 1;
}