#ifndef GUARD_MENU_SHORTSTRINGENTRY_FRAGMENTS_CHARSETBUTTONS
#define GUARD_MENU_SHORTSTRINGENTRY_FRAGMENTS_CHARSETBUTTONS

#include "lu/vui/sprite-button.h"
#include "menus/short_string_entry/charsets.h"

union ShortStringEntryMenuCharsetButtons {
   VUISpriteButton list[5];
   struct {
      VUISpriteButton upper;
      VUISpriteButton lower;
      VUISpriteButton symbol;
      VUISpriteButton accent_u;
      VUISpriteButton accent_l;
   };
};

extern void ShortStringEntryMenu_SetUpCharsetButtons(union ShortStringEntryMenuCharsetButtons*);

extern void ShortStringEntryMenu_UpdateSelectedCharsetButtonSprite(
   union ShortStringEntryMenuCharsetButtons*,
   enum ShortStringEntryMenu_Charset selected
);

//
// TODO: We also need a way to indicate which button the cursor is over.
//

#endif