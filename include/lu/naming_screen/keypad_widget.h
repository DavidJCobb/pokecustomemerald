#ifndef GUARD_LU_NAMING_SCREEN_KEYPAGE_H
#define GUARD_LU_NAMING_SCREEN_KEYPAGE_H

#include "gba/types.h"

struct LuNamingScreenKeypadWidget {
   struct {
      u8 x;
      u8 y;
   } pos; // in tiles
   u8 charset;
   u8 bg;
   u8 window;
   u8 palette;
   u8 cursor_sprite;
   struct {
      u8 x; // measured in cols
      u8 y; // measured in rows
   } cursor_pos;
};

extern void InitNamingScreenKeypad(struct LuNamingScreenKeypadWidget*);


enum LuNamingScreenKeypadInputResultType {
   NAMINGSCREENKEYPAD_INPUT_NONE = 0,
   NAMINGSCREENKEYPAD_INPUT_CURSOR,
   NAMINGSCREENKEYPAD_INPUT_APPEND,
   NAMINGSCREENKEYPAD_INPUT_BACKSPACE,
   NAMINGSCREENKEYPAD_INPUT_SUBMIT,
};
struct LuNamingScreenKeypadInputResult {
   enum LuNamingScreenKeypadInputResultType type;
   u8 character;
};
extern void NamingScreenKeypad_HandleInput(
   struct LuNamingScreenKeypadWidget*,
   struct LuNamingScreenKeypadInputResult*
);

#endif