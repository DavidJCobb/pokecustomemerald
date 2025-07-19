#ifndef GUARD_LU_V_WIDGET_KEYBOARD_H
#define GUARD_LU_V_WIDGET_KEYBOARD_H

#include "lu/v-widgets/v-widgets.h"

struct LuVWidget_Keyboard_InitParams {
   u8  bg_layer : 2;
   u8  palette  : 4;
   u8  color_background  : 4;
   u8  color_text_fill   : 4;
   u8  color_text_shadow : 4;
   u8  tile_x;
   u8  tile_y;
   u16 first_tile_id; // char-based tile ID
};

struct LuVWidget_Keyboard {
   struct LuVWidget base;
   
   // Configuration:
   struct {
      void(*on_text_changed)(const u8* buffer);
      void(*on_text_at_maxlength)(void);
   } callbacks;
   struct {
      u8  bg_layer : 2;
      u8  palette  : 4;
      u8  color_background  : 4;
      u8  color_text_fill   : 4;
      u8  color_text_shadow : 4;
      u8  window_id : 4;
   } rendering;
   struct {
      u8* buffer;     // always EOS-terminated
      u8  max_length; // not including EOS, e.g. max length of a Pokemon name is 10 in vanilla
   } value;
   u8 charset;
   struct {
      u8 sprite_id;
      u8 row;
      u8 col;
   } cursor;
   struct {
      //
      // If the user moves the cursor all the way to the right, within the 
      // keyboard, and then presses right once more, then we want to move 
      // the cursor out of the control and focus one of the buttons to the 
      // right... but which button? Whichever the cursor (by virtue of its 
      // Y-position) is nearest to.
      //
      // So, as the cursor moves up and down within the keyboard, we'll want 
      // to change the keyboard's `base.navigation.id_right`.
      //
      u8 ids_by_row[3]; // from top to bottom
   } sized_navigation;
};

extern void LuVWidget_Keyboard_InitBase(struct LuVWidget_Keyboard*, const struct LuVWidget_Keyboard_InitParams*);

#endif