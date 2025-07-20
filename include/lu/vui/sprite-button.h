#ifndef GUARD_LU_VUI_SPRITEBUTTON_H
#define GUARD_LU_VUI_SPRITEBUTTON_H

#include "lu/vui/vui-widget.h"

typedef struct VUISpriteButton {
   VUI_WIDGET_SUBCLASS_HEADER(VUIWidget);
   void(*on_press)(void);
   u8  sprite_id;
   u16 state_data_index; // gSprites[sprite_id].data[state_data_index] = state
} VUISpriteButton;

extern void VUISpriteButton_Construct(VUISpriteButton*);
extern void VUISpriteButton_TakeSprite(VUISpriteButton*, u8 sprite_id);

#endif