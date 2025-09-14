#ifndef GUARD_LU_VUI_SPRITEBUTTON_H
#define GUARD_LU_VUI_SPRITEBUTTON_H

#include "lu/vui/vui-widget.h"
#include "lu/c-attr.define.h"

struct VUISpriteButton_Callbacks {
   void(*on_press)(void);
};

typedef struct VUISpriteButton_InitParams {
   struct VUISpriteButton_Callbacks callbacks;
   VUIGridArea grid;
} VUISpriteButton_InitParams;

typedef struct VUISpriteButton {
   VUI_WIDGET_SUBCLASS_HEADER(VUIWidget);
   struct VUISpriteButton_Callbacks callbacks;
   u8  sprite_id;
   u16 state_data_index; // gSprites[sprite_id].data[state_data_index] = state
} VUISpriteButton;

NON_NULL_PARAMS(1) extern void VUISpriteButton_Construct(VUISpriteButton*);
NON_NULL_PARAMS(1,2) extern void VUISpriteButton_Initialize(VUISpriteButton*, const VUISpriteButton_InitParams*);
NON_NULL_PARAMS(1) extern void VUISpriteButton_TakeSprite(VUISpriteButton*, u8 sprite_id);

#include "lu/c-attr.undef.h"
#endif