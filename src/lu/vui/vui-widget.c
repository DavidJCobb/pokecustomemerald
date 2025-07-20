#include "lu/vui/vui-widget.h"

static const struct VTable_VUIWidget sNullVTable_Widget = {0};
/*extern*/ const struct VTable_VUIWidget gVTable_VUIWidget = {0};

extern void VUIWidget_Construct(VUIWidget* this) {
   *this = (VUIWidget){0};
   this->functions = &sNullVTable_Widget;
   this->size      = (VUISize){ 1, 1 };
}
extern void VUIWidget_Destroy(VUIWidget* this) {
   if (!this)
      return;
   v_invoke(this, _impl_destroy)();
   this->functions = &sNullVTable_Widget;
   this->disabled  = TRUE;
   this->focusable = TRUE;
}
extern bool8 VUIWidget_IsFocusable(const VUIWidget* this) {
   return this && this->focusable && !this->disabled;
}