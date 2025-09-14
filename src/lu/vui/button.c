#include "lu/vui/button.h"
#include "gba/defines.h"
#include "gba/isagbprint.h"
#include "main.h" // gMain (referenced by JOY_NEW and friends)
#include "global.h" // JOY_NEW and friends

static u8 VFunc_DestroyImpl(VUIWidget*);
static u8 VFunc_OnFrame(VUIWidget*);
//
const struct VTable_VUIWidget gVTable_VUIButton = {
   &gVTable_VUIWidget,
   VFunc_DestroyImpl,
   VFunc_OnFrame,
   NULL,
};

static void Repaint(VUIButton*, bool8 is_focused);

extern void VUIButton_Construct(VUIButton* this) {
   VUIWidget_Construct(&this->base);
   this->base.functions = &gVTable_VUIButton;
   
}
extern void VUIButton_Initialize(VUIButton* this, const VUIButton_InitParams* params) {
   this->base.focusable = TRUE;
   VUIWidget_SetGridMetrics(this, params->grid.pos.x, params->grid.pos.y, params->grid.size.w, params->grid.size.h);
   
   this->callbacks = params->callbacks;
}

static u8 VFunc_DestroyImpl(VUIWidget* widget) {
   VUIButton* this = (VUIButton*)widget;
}
static u8 VFunc_OnFrame(VUIWidget* widget) {
   VUIButton* this = (VUIButton*)widget;
   if (JOY_NEW(A_BUTTON)) {
      if (this->callbacks.on_press)
         (this->callbacks.on_press)();
      return VWIDGET_FRAMEHANDLER_CONSUMED_DPAD;
   }
   return 0;
}