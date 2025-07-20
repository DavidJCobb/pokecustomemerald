#include "lu/vui/sprite-button.h"
#include "gba/isagbprint.h"
#include "global.h" // JOY_NEW and friends
#include "main.h"
#include "sprite.h"

static u8 VFunc_DestroyImpl(VUIWidget*);
static u8 VFunc_OnFrame(VUIWidget*);
static void VFunc_OnFocusChange(VUIWidget*, bool8, VUIWidget*);
//
static const struct VTable_VUIWidget sVTable = {
   &gVTable_VUIWidget,
   VFunc_DestroyImpl,
   VFunc_OnFrame,
   VFunc_OnFocusChange,
};

extern void VUISpriteButton_Construct(VUISpriteButton* this) {
   VUIWidget_Construct(&this->base);
   this->base.functions   = &sVTable;
   this->base.focusable   = TRUE;
   this->on_press         = NULL;
   this->sprite_id        = SPRITE_NONE;
   this->state_data_index = 7;
}
extern void VUISpriteButton_TakeSprite(VUISpriteButton* this, u8 sprite_id) {
   if (this->sprite_id != SPRITE_NONE) {
      DestroySprite(&gSprites[this->sprite_id]);
   }
   this->sprite_id = sprite_id;
}

static u8 VFunc_DestroyImpl(VUIWidget* widget) {
   VUISpriteButton* this = (VUISpriteButton*)widget;
   if (this->sprite_id != SPRITE_NONE) {
      DestroySprite(&gSprites[this->sprite_id]);
      this->sprite_id = SPRITE_NONE;
   }
}
static u8 VFunc_OnFrame(VUIWidget* widget) {
   VUISpriteButton* this = (VUISpriteButton*)widget;
   if (JOY_NEW(A_BUTTON)) {
      if (this->on_press)
         (this->on_press)();
      return VWIDGET_FRAMEHANDLER_CONSUMED_DPAD;
   }
   return 0;
}
static void VFunc_OnFocusChange(VUIWidget* widget, bool8 gained, VUIWidget*) {
   VUISpriteButton* this = (VUISpriteButton*)widget;
   if (this->sprite_id == SPRITE_NONE)
      return;
   AGB_ASSERT(this->state_data_index < 8);
   
   u16 state = 0;
   if (gained)
      state |= 1;
   if (widget->disabled)
      state |= 2;
   
   gSprites[this->sprite_id].data[this->state_data_index] = state;
}