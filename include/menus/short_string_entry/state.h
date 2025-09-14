#ifndef GUARD_MENU_SHORTSTRINGENTRY_STATE
#define GUARD_MENU_SHORTSTRINGENTRY_STATE

#include "gba/defines.h" // BG_SCREEN_SIZE
#include "gba/types.h"
#include "lu/vui/vui-context.h"
#include "lu/vui/custom-keyboard.h"
#include "lu/vui/keyboard-value.h"
#include "lu/vui/sprite-button.h"
#include "lu/vui/button.h"
#include "./fragments/charset_buttons.h"
#include "./params.h"

struct ShortStringEntryMenuParams;

struct ShortStringEntryMenuState {
   void(*callback)(const u8*);
   u8 buffer[VUIKEYBOARDVALUE_MAX_SUPPORTED_SIZE + 1];
   
   struct ShortStringEntryMenuIcon icon;
   const u8* title; // optional
   
   struct {
      VUIContext context;
      struct {
         VUICustomKeyboard keyboard;
         VUIKeyboardValue  value;
         VUIButton         button_ok;
         VUIButton         button_backspace;
         union ShortStringEntryMenuCharsetButtons charset_buttons;
      } widgets;
      VUIWidget* widget_list[9];
   } vui;
   
   union {
      u8 all[7];
      struct {
         u8 icon;
         u8 cursor_menu_button;
         u8 cursor_charset_button_sprites[5]; // one for each button
      };
   } sprite_ids;
   union {
      u8 all[4];
      struct {
         u8 gender;
         u8 title;
         u8 button_label_ok;
         u8 button_label_backspace;
      };
   } window_ids;
   
   u8 task_id;
   u8 timer;
   u8 gender;
   u8 max_length;
   
   u8 tilemap_buffers[4][BG_SCREEN_SIZE];
};

extern struct ShortStringEntryMenuState* gShortStringEntryMenuState;

// Creates the state object, and links the VUI context to the widgets.
extern void ShortStringEntryMenu_CreateState(void);

extern void ShortStringEntryMenu_InitState(const struct ShortStringEntryMenuParams*);

extern void ShortStringEntryMenu_DestroyState(void);

#endif