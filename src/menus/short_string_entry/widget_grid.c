#include "menus/short_string_entry/widget_grid.h"
#include "menus/short_string_entry/state.h"
#include "lu/c.h"

// The abstract-grid layout for our VUI context.
enum {
   /*
      +-------+-------------------------+--------+
      |                                 |   OK   |
      |                Keyboard         +--------+
      |                                 |  Del.  |
      +-------+-------+--------+--------+--------+
      | Upper | Lower | Symbol | ACCENT | accent |
      +-------+-------+--------+--------+--------+
   
   */
   CTXGRID_KEYBOARD_X = 0,
   CTXGRID_KEYBOARD_Y = 0,
   CTXGRID_KEYBOARD_W = 4,
   CTXGRID_KEYBOARD_H = 3,
   
   CTXGRID_BUTTON_OK_X = 4,
   CTXGRID_BUTTON_OK_Y = CTXGRID_KEYBOARD_Y,
   CTXGRID_BUTTON_OK_W = 1,
   CTXGRID_BUTTON_OK_H = CTXGRID_KEYBOARD_H / 2,
   
   CTXGRID_BUTTON_BACK_X = CTXGRID_BUTTON_OK_X,
   CTXGRID_BUTTON_BACK_Y = CTXGRID_BUTTON_OK_Y + CTXGRID_BUTTON_OK_H,
   CTXGRID_BUTTON_BACK_W = CTXGRID_BUTTON_OK_W,
   CTXGRID_BUTTON_BACK_H = CTXGRID_KEYBOARD_H - CTXGRID_BUTTON_BACK_Y,
   
   CTXGRID_CHARSETBUTTON_UPPER_X       = 0,
   CTXGRID_CHARSETBUTTON_LOWER_X       = 1,
   CTXGRID_CHARSETBUTTON_SYMBOL_X      = 2,
   CTXGRID_CHARSETBUTTON_ACCENTUPPER_X = 3,
   CTXGRID_CHARSETBUTTON_ACCENTLOWER_X = 4,
   CTXGRID_CHARSETBUTTON_Y = CTXGRID_KEYBOARD_Y + CTXGRID_KEYBOARD_H,
   
   CTXGRID_W = CTXGRID_BUTTON_OK_X + CTXGRID_BUTTON_OK_W,
   CTXGRID_H = CTXGRID_CHARSETBUTTON_Y + 1,
};

extern void ShortStringEntryMenu_SetUpWidgetGrid(struct ShortStringEntryMenuState* state) {
   VUIContext* context = &state->vui.context;
   context->w = CTXGRID_W;
   context->h = CTXGRID_H;
   
   VUIWidget_SetGridMetrics(
      &state->vui.widgets.keyboard,
      CTXGRID_KEYBOARD_X,
      CTXGRID_KEYBOARD_Y,
      CTXGRID_KEYBOARD_W,
      CTXGRID_KEYBOARD_H
   );
   VUIWidget_SetGridMetrics(
      &state->vui.widgets.button_ok,
      CTXGRID_BUTTON_OK_X,
      CTXGRID_BUTTON_OK_Y,
      CTXGRID_BUTTON_OK_W,
      CTXGRID_BUTTON_OK_H
   );
   VUIWidget_SetGridMetrics(
      &state->vui.widgets.button_backspace,
      CTXGRID_BUTTON_BACK_X,
      CTXGRID_BUTTON_BACK_Y,
      CTXGRID_BUTTON_BACK_W,
      CTXGRID_BUTTON_BACK_H
   );
   {
      auto list = &state->vui.widgets.charset_buttons.list[0];
      for(int i = 0; i < 5; ++i) {
         auto widget = &list[i];
         VUIWidget_SetGridMetrics(
            widget,
            CTXGRID_CHARSETBUTTON_UPPER_X + i,
            CTXGRID_CHARSETBUTTON_Y,
            1,
            1
         );
      }
   }
}