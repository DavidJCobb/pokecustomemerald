#include "lu/v-widgets/sprite-button.h"
#include "gba/isagbprint.h"
#include "main.h"
#include "sprite.h"

static u8 VFunc_DestroyImpl(struct LuVWidget*);
static u8 VFunc_OnFrame(struct LuVWidget*);
static void VFunc_OnFocusChange(struct LuVWidget*, bool8, struct LuVWidget*);
//
const struct LuVWidgetVTable sVTable_SpriteButton = {
   VFunc_DestroyImpl,
   VFunc_OnFrame,
   VFunc_OnFocusChange,
};

extern void LuVWidget_SpriteButton_InitBase(struct LuVWidget_SpriteButton* widget) {
   widget->base.functions = &sVTable_SpriteButton;
   widget->focusable = TRUE;
   widget->sprite_id = SPRITE_NONE;
   if (widget->state_data_index > 7) { // uninitialized memory
      widget->state_data_index = 7;
   }
}
extern void LuVWidget_SpriteButton_TakeSprite(struct LuVWidget_SpriteButton* widget, u8 sprite_id) {
   if (widget->sprite_id != SPRITE_NONE) {
      DestroySprite(&gSprites[widget->sprite_id]);
   }
   widget->sprite_id = sprite_id;
}

static u8 VFunc_DestroyImpl(struct LuVWidget* widget) {
   struct LuVWidget_SpriteButton* casted = (struct LuVWidget_SpriteButton*)widget;
   if (casted->sprite_id != SPRITE_NONE) {
      DestroySprite(&gSprites[casted->sprite_id]);
      casted->sprite_id = SPRITE_NONE;
   }
}
static u8 VFunc_OnFrame(struct LuVWidget* widget) {
   struct LuVWidget_SpriteButton* casted = (struct LuVWidget_SpriteButton*)widget;
   if (JOY_NEW(A_BUTTON)) {
      if (widget->on_press)
         (widget->on_press)();
      return VWIDGET_FRAMEHANDLER_CONSUMED_DPAD;
   }
   return 0;
}
static void VFunc_OnFocusChange(struct LuVWidget* widget, bool8 gained, struct LuVWidget*) {
   struct LuVWidget_SpriteButton* casted = (struct LuVWidget_SpriteButton*)widget;
   if (casted->sprite_id == SPRITE_NONE)
      return;
   AGB_ASSERT(casted->state_data_index < 8);
   
   u16 state = 0;
   if (gained)
      state |= 1;
   if (widget->disabled)
      state |= 2;
   
   gSprites[casted->sprite_id].data[casted->state_data_index] = state;
}