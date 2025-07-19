#include "lu/v-widgets/keyboard.h"
#include "main.h"
#include "sprite.h"
#include "window.h"

enum {
   COL_COUNT = 10,
   ROW_COUNT  = 4,
};

static u8 VFunc_DestroyImpl(struct LuVWidget*);
static u8 VFunc_OnFrame(struct LuVWidget*);
static void VFunc_OnFocusChange(struct LuVWidget*, bool8, struct LuVWidget*);
//
const struct LuVWidgetVTable sVTable_Keyboard = {
   VFunc_DestroyImpl,
   VFunc_OnFrame,
   VFunc_OnFocusChange,
};

enum {
   CHARSET_UPPER,
   CHARSET_LOWER,
   CHARSET_SYMBOLS,
};
static const struct CharsetInfo {
   const u8* characters;
   u8 cols;
} sCharsets[] = {
   {
      .characters = __(
         "ABC DEF  ."
         "GHI JKL  ,"
         "MNO PRQS  "
         "TUV WXYZ  "
      ),
      .cols = 10
   },
   {
      .characters = __(
         "abc def  ."
         "ghi jkl  ,"
         "mno pqrs  "
         "tuv wxyz  "
      ),
      .cols = 10
   },
   {
      .characters = __(
         "01234  "
         "56789  "
         "!?♂♀/- "
         "…“”‘'  "
      ),
      .cols = 7
   },
};

static u8 GetColCount(const struct LuVWidget_Keyboard*);
static void MoveCursor(struct LuVWidget_Keyboard*, s8 x, s8 y);
static void AppendGlyph(struct LuVWidget_Keyboard*);
static void RepaintKeys(struct LuVWidget_Keyboard*);

// --------------------------------------------------------------------------------

extern void LuVWidget_Keyboard_InitBase(
   struct LuVWidget_Keyboard* widget,
   const struct LuVWidget_Keyboard_InitParams* params
) {
   widget->base.functions = &sVTable_SpriteButton;
   widget->focusable = TRUE;
   
   widget->callbacks = {0};
   
   widget->window_id = WINDOW_NONE;
   
   widget->value.buffer     = NULL;
   widget->value.max_length = 0;
   widget->charset = 0;
   widget->cursor.sprite_id = SPRITE_NONE;
   widget->cursor.row = 0;
   widget->cursor.col = 0;
   widget->sized_navigation = {0};
   
   widget->rendering.bg_layer = params->bg_layer;
   widget->rendering.palette  = params->palette;
   widget->rendering.color_background  = params->color_background;
   widget->rendering.color_text_fill   = params->color_text_fill;
   widget->rendering.color_text_shadow = params->color_text_shadow;
   
   //
   // TODO: Create cursor sprite
   //
   {
      const struct WindowTemplate tmpl = {
         .bg          = params->bg_layer,
         .tilemapLeft = params->tile_x + 1,
         .tilemapTop  = params->tile_y + 1,
         .width       = COL_COUNT * 2,
         .height      = ROW_COUNT * 2,
         .paletteNum  = params->palette,
         .baseBlock   = params->first_tile_id
      };
      u8 window_id = AddWindow(&tmpl);
      AGB_ASSERT(window_id != WINDOW_NONE);
      widget->rendering.window_id = window_id;
      
      PutWindowTilemap(window_id);
      FillWindowPixelBuffer(window_id, PIXEL_FILL(widget->rendering.color_background));
   }
   
   RepaintKeys(widget);
}

// --------------------------------------------------------------------------------

static u8 VFunc_DestroyImpl(struct LuVWidget* widget) {
   struct LuVWidget_Keyboard* this = (struct LuVWidget_Keyboard*)widget;
   if (this->cursor.sprite_id != SPRITE_NONE) {
      DestroySprite(&gSprites[this->cursor.sprite_id]);
      this->cursor.sprite_id = SPRITE_NONE;
   }
   if (this->window_id != WINDOW_NONE) {
      RemoveWindow(this->window_id);
      this->window_id = WINDOW_NONE;
   }
}

static u8 VFunc_OnFrame(struct LuVWidget* widget) {
   struct LuVWidget_Keyboard* this = (struct LuVWidget_Keyboard*)widget;
   if (JOY_NEW(A)) {
      AppendGlyph(this);
      return VWIDGET_FRAMEHANDLER_CONSUMED_DPAD;
   }
   
   s8 move_x = 0;
   s8 move_y = 0;
   if (JOY_NEW(DPAD_LEFT)) {
      move_x = -1;
   } else if (JOY_NEW(DPAD_RIGHT)) {
      move_x = 1;
   } else if (JOY_NEW(DPAD_UP)) {
      move_y = -1;
   } else if (JOY_NEW(DPAD_DOWN)) {
      move_y = 1;
   }
   
   if (
      (move_x < 0 && this->cursor.col == 0)
   || (move_x > 0 && this->cursor.col == GetColCount(this))
   ) {
      return 0;
   }
   MoveCursor(this, move_x, move_y);
   return VWIDGET_FRAMEHANDLER_CONSUMED_DPAD;
}

static void VFunc_OnFocusChange(struct LuVWidget* widget, bool8 gained, struct LuVWidget* other) {
   struct LuVWidget_Keyboard* this = (struct LuVWidget_Keyboard*)widget;
   if (gained) {
      if (other) {
         if (other->id == this->sized_navigation.ids_by_row[0])
            this->cursor.row = 0;
         else if (other->id == this->sized_navigation.ids_by_row[1]) {
            if (this->cursor.row < 1)
               this->cursor.row = 1;
            else if (this->cursor.row > 2)
               this->cursor.row = 2;
         } else if (other->id == this->sized_navigation.ids_by_row[2])
            this->cursor.row = 3;
      }
   }
}

static u8 GetColCount(const struct LuVWidget_Keyboard* this) {
   u8 cols = sCharsets[this->charset].cols;
   if (cols == 0)
      cols = 8;
   return cols;
}
static void MoveCursor(struct LuVWidget_Keyboard* this, s8 x, s8 y) {
   if (x) {
      u8 col_count = GetColCount(this);
      this->cursor.col += x;
      if (this->cursor.col >= col_count)
         this->cursor.col %= col_count;
   }
   if (y) {
      this->cursor.row += y;
      if (this->cursor.row >= ROW_COUNT) {
         this->cursor.row %= ROW_COUNT;
      }
      //
      // Update horizontal cross-widget navigation.
      //
      u8 id = this->base.navigation.id_left;
      switch (this->cursor.row) {
         case 0:
            id = this->sized_navigation.ids_by_row[0];
            break;
         case 1:
         case 2:
            id = this->sized_navigation.ids_by_row[1];
            break;
         case 3:
            id = this->sized_navigation.ids_by_row[2];
            break;
      }
      this->base.navigation.id_left = this->base.navigation.id_right = id;
   }
   //
   // Update sprite.
   //
   if (this->cursor.sprite_id != SPRITE_NONE) {
      struct Sprite* sprite = &gSprites[this->cursor.sprite_id];
      sprite->x2 = this->cursor.col * TILE_WIDTH  * 2;
      sprite->y2 = this->corsor.row * TILE_HEIGHT * 2;
   }
}
static void AppendGlyph(struct LuVWidget_Keyboard* this) {
   if (!this->value.buffer || !this->value.max_length)
      return;
   if (this->value.buffer[this->value.max_length - 1] != EOS) {
      if (this->callbacks.on_text_at_maxlength)
         (this->callbacks.on_text_at_maxlength)();
      return;
   }
   
   u8 character = 0;
   {
      u8 col_count = GetColCount(this);
      u8 i = this->cursor.row * col_count + this->cursor.col;
      character = sCharsets[this->charset].characters[i];
   }
   u8 i = 0;
   for(; i < this->value.max_length; ++i)
      if (this->value.buffer[i] == EOS)
         break;
   this->value[i] = character;
   if (this->callbacks.on_text_changed)
      (this->callbacks.on_text_changed)(this->value.buffer);
   if (i == this->value.max_length - 1)
      if (this->callbacks.on_text_at_maxlength)
         (this->callbacks.on_text_at_maxlength)();
}
static void RepaintKeys(struct LuVWidget_Keyboard* this) {
   _Static_assert(false, "TODO");
}