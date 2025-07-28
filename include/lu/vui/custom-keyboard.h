#ifndef GUARD_LU_VUI_CUSTOM_KEYBOARD_H
#define GUARD_LU_VUI_CUSTOM_KEYBOARD_H

#include "gba/defines.h"
#include "lu/vui/vui-widget.h"
#include "lu/vui/keyboard.h"
struct VUIFrame;

struct VUICustomKeyboardCharset {
   const u8* characters;
   u8 rows;
   u8 cols;
   struct {
      u8 count;
      u8 positions[5];
   } col_gaps;
};
extern const struct VUICustomKeyboardCharset gVUICustomKeyboardDefaultCharsets[3];

typedef struct VUICustomKeyboard_InitParams {
   VUIStringRef buffer;
   struct VUIKeyboard_Callbacks callbacks;
   const struct VUICustomKeyboardCharset* charsets;
   u8 charsets_count;
   const struct VUIFrame* frame;
   VUIGridArea grid;
   u8  bg_layer : 2;
   u8  palette  : 4;
   VUITextColors colors;
   u8  tile_x;
   u8  tile_y;
   u16 first_tile_id; // char-based tile ID
} VUICustomKeyboard_InitParams;

typedef struct VUICustomKeyboard {
   VUI_WIDGET_SUBCLASS_HEADER(VUIWidget);
   struct VUIKeyboard_Callbacks callbacks;
   const struct VUICustomKeyboardCharset* charsets;
   u8 charsets_count;
   VUITextColors colors;
   struct {
      u8  bg_layer : 2;
      u8  palette  : 4;
   } rendering;
   VUIStringRef value;
   u8 charset;
   u8 cursor_sprite_id;
   u8 window_id;
} VUICustomKeyboard;

extern void VUICustomKeyboard_Construct(VUICustomKeyboard*, const VUICustomKeyboard_InitParams*);
extern void VUICustomKeyboard_Backspace(VUICustomKeyboard*);
extern void VUICustomKeyboard_NextCharset(VUICustomKeyboard*);
extern void VUICustomKeyboard_SetCharset(VUICustomKeyboard*, u8);

#endif