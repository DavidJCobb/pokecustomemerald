#ifndef GUARD_LU_VUI_KEYBOARDVALUE_H
#define GUARD_LU_VUI_KEYBOARDVALUE_H

#include "lu/vui/vui-widget.h"
struct VUIFrame;

enum {
   VUIKEYBOARDVALUE_SPRITE_GRAPHIC_TAG = 0x5000,
   VUIKEYBOARDVALUE_SPRITE_PALETTE_TAG = 0x5000,
   VUIKEYBOARDVALUE_MAX_SUPPORTED_SIZE = 10,
   VUIKEYBOARDVALUE_WINDOW_TILE_COUNT  = (VUIKEYBOARDVALUE_MAX_SUPPORTED_SIZE*2) * 2,
};

typedef struct VUIKeyboardValue_InitParams {
   u8  bg_layer : 2;
   u8  palette  : 4;
   VUITextColors colors;
   const struct VUIFrame* frame;
   u8  tile_x;
   u8  tile_y;
   u16 first_tile_id; // char-based tile ID
   u8  max_length;
} VUIKeyboardValue_InitParams;

typedef struct VUIKeyboardValue {
   VUI_WIDGET_SUBCLASS_HEADER(VUIWidget);
   VUITextColors colors;
   struct {
      u8  bg_layer  : 2;
      u8  palette   : 4;
      u8  window_id;
   } rendering;
   u8 max_length;
   u8 underscore_sprite_ids[VUIKEYBOARDVALUE_MAX_SUPPORTED_SIZE];
} VUIKeyboardValue;

extern void VUIKeyboardValue_Construct(VUIKeyboardValue*, const VUIKeyboardValue_InitParams*);
extern void VUIKeyboardValue_SetUnderscoreVisibility(VUIKeyboardValue*, bool8);
extern void VUIKeyboardValue_ShowValue(VUIKeyboardValue*, const u8* string);

#endif