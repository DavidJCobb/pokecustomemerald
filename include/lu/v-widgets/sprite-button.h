#ifndef GUARD_LU_V_WIDGET_SPRITEBUTTON_H
#define GUARD_LU_V_WIDGET_SPRITEBUTTON_H

#include "lu/v-widgets/v-widgets.h"

struct LuVWidget_SpriteButton {
   struct LuVWidget base;
   void(*on_press)(void);
   u8  sprite_id;
   u16 state_data_index; // gSprites[sprite_id].data[state_data_index] = state
};

extern void LuVWidget_SpriteButton_InitBase(struct LuVWidget_SpriteButton*);
extern void LuVWidget_SpriteButton_TakeSprite(struct LuVWidget_SpriteButton*, u8 sprite_id);

#endif