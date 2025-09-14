#ifndef GUARD_LU_VUI_KEYBOARD_H
#define GUARD_LU_VUI_KEYBOARD_H

#include "gba/defines.h"
#include "lu/vui/vui-widget.h"
#include "lu/c-attr.define.h"

enum VUIKeyboardCharsetID {
   VUIKEYBOARD_CHARSET_UPPER,
   VUIKEYBOARD_CHARSET_LOWER,
   VUIKEYBOARD_CHARSET_SYMBOLS,
   //
   VUIKEYBOARD_CHARSET_COUNT
};

#define PX_TO_TILES(n) (((n) / TILE_WIDTH) + (((n) % TILE_WIDTH) ? 1 : 0))
#define ROUND_TO_TILES(n) ((n) + ((n) % TILE_WIDTH ? 1 : 0) * TILE_WIDTH)
enum {
   VUIKEYBOARD_SPRITE_GRAPHIC_TAG = 0x5001,
   VUIKEYBOARD_SPRITE_PALETTE_TAG = 0x5001,
   
   VUIKEYBOARD_MAX_GRID_COLS   = 12,
   VUIKEYBOARD_MAX_GRID_ROWS   =  4,
   
   VUIKEYBOARD_PX_EXTRA_SPACING_FLAT    = 8,
   VUIKEYBOARD_PX_EXTRA_SPACING_PER_COL = 0,
   VUIKEYBOARD_PX_COL_W                 = TILE_WIDTH,
   VUIKEYBOARD_PX_INNER_W               = ROUND_TO_TILES(
      ((VUIKEYBOARD_MAX_GRID_COLS - 1) * VUIKEYBOARD_PX_EXTRA_SPACING_PER_COL)
    + (VUIKEYBOARD_MAX_GRID_COLS * VUIKEYBOARD_PX_COL_W)
    + VUIKEYBOARD_PX_EXTRA_SPACING_FLAT
   ),
   
   VUIKEYBOARD_PX_ROW_H   = TILE_HEIGHT * 2,
   VUIKEYBOARD_PX_INNER_H = VUIKEYBOARD_MAX_GRID_ROWS * VUIKEYBOARD_PX_ROW_H,
   
   VUIKEYBOARD_INNER_W_TILES      = PX_TO_TILES(VUIKEYBOARD_PX_INNER_W),
   VUIKEYBOARD_INNER_H_TILES      = PX_TO_TILES(VUIKEYBOARD_PX_INNER_H),
   VUIKEYBOARD_WINDOW_TILE_COUNT  = VUIKEYBOARD_INNER_W_TILES * VUIKEYBOARD_INNER_H_TILES,
};
#undef PX_TO_TILES

struct VUIKeyboard_Callbacks {
   void(*on_text_changed)(const u8* buffer);
   void(*on_text_at_maxlength)(void);
};

typedef struct VUIKeyboard_InitParams {
   VUIStringRef buffer;
   struct VUIKeyboard_Callbacks callbacks;
   VUIGridArea grid;
   u8  bg_layer : 2;
   u8  palette  : 4;
   VUITextColors colors;
   u8  tile_x;
   u8  tile_y;
   u16 first_tile_id; // char-based tile ID
} VUIKeyboard_InitParams;

typedef struct VUIKeyboard {
   VUI_WIDGET_SUBCLASS_HEADER(VUIWidget);
   struct VUIKeyboard_Callbacks callbacks;
   VUITextColors colors;
   struct {
      u8  bg_layer : 2;
      u8  palette  : 4;
      u8  window_id;
   } rendering;
   VUIStringRef value;
   u8 charset;
   u8 cursor_sprite_id;
} VUIKeyboard;

NON_NULL_PARAMS(1)
extern void VUIKeyboard_Construct(VUIKeyboard*);
NON_NULL_PARAMS(1,2)
extern void VUIKeyboard_Initialize(VUIKeyboard*, const VUIKeyboard_InitParams*);
NON_NULL_PARAMS(1) extern void VUIKeyboard_Backspace(VUIKeyboard*);
NON_NULL_PARAMS(1) extern void VUIKeyboard_NextCharset(VUIKeyboard*);
NON_NULL_PARAMS(1) extern void VUIKeyboard_SetCharset(VUIKeyboard*, enum VUIKeyboardCharsetID);

#include "lu/c-attr.undef.h"
#endif