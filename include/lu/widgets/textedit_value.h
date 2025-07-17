#ifndef GUARD_LU_UI_WIDGET_TEXTEDIT_VALUE
#define GUARD_LU_UI_WIDGET_TEXTEDIT_VALUE

#include "gba/types.h"
#include "string_util.h" // enum StringConvertMode

struct LuTexteditValueWidget {
   u8 cursor_pos;
   u8 max_length : 4;
   bool8 owns_window : 1;
   
   u8 window;
   u8 win_x;
   u8 win_y;
   struct {
      u8 palette_id : 4;
      u8 bg         : 4;
      u8 text       : 4;
      u8 shadow     : 4;
   } color;
   
   u8 underscore_sprite_ids[15];
};

struct LuTexteditValueWidgetInitParams {
   bool8 use_own_window;
   
   u8 max_length : 4;
   union {
      struct {
         u8 bg_layer;
         u8 first_tile_id;
         u8 x; // window position in tiles
         u8 y; // window position in tiles
      } window_to_create;
      struct {
         u8 window_id;
         u8 x; // offset within the window, to draw to, in px
         u8 y; // offset within the window, to draw to, in px
      } window_to_share;
   };
   struct {
      u8 palette_id;
      u8 bg;
      u8 text;
      u8 shadow;
   } color;
   
   // Tags uniquely identify resources used by a sprite. If two sprites 
   // with the same tile tag load, then they will share sprite tiles in
   // VRAM; and thus also for the palette tag.
   struct {
      u16 tile;
      u16 palette;
   } sprite_tags;
};

extern void InitTexteditValueWidget(struct LuTexteditValueWidget*, const struct LuTexteditValueWidgetInitParams*);

extern void ShowTexteditValueWidget(struct LuTexteditValueWidget*);

extern void PaintTexteditValueNumber(struct LuTexteditValueWidget*, u32 number, enum StringConvertMode);
extern void PaintTexteditValueString(struct LuTexteditValueWidget*, const u8* string);
extern void SetTexteditValueCursorPos(struct LuTexteditValueWidget*, u8);

extern void DestroyTexteditValueWidget(struct LuTexteditValueWidget*);

#endif