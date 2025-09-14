#ifndef GUARD_LU_VUI_BUTTON_H
#define GUARD_LU_VUI_BUTTON_H

#include "lu/vui/vui-widget.h"
#include "lu/c-attr.define.h"

extern const struct VTable_VUIWidget gVTable_VUIButton;

struct VUIButton_Callbacks {
   void(*on_press)(void);
};

typedef struct VUIButton_InitParams {
   VUIGridArea grid;
   struct VUIButton_Callbacks callbacks;
} VUIButton_InitParams;

typedef struct VUIButton {
   VUI_WIDGET_SUBCLASS_HEADER(VUIWidget);
   struct VUIButton_Callbacks callbacks;
} VUIButton;

NON_NULL_PARAMS(1) extern void VUIButton_Construct(VUIButton*);
NON_NULL_PARAMS(1,2) extern void VUIButton_Initialize(VUIButton*, const VUIButton_InitParams*);

#include "lu/c-attr.undef.h"
#endif