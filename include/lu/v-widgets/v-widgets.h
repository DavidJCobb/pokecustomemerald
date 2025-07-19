#ifndef GUARD_LU_V_WIDGETS_H
#define GUARD_LU_V_WIDGETS_H

#include "gba/types.h"

// Frame-handler flags, to be returned by a v-widget's `on_frame` handler.
enum {
   VWIDGET_FRAMEHANDLER_CONSUMED_DPAD,
};

struct LuVWidget;

struct LuVWidgetVTable {
   u8(*impl_destroy)(struct LuVWidget* this); // don't invoke directly; call LuVWidget_Destroy
   u8(*on_frame)(struct LuVWidget* this);
   void(*on_focus_change)(struct LuVWidget* this, bool8 gained, struct LuVWidget*);
};

struct LuVWidget {
   struct LuWidgetVTable* functions; // must not be NULL
   u8 id : 6; // must be non-zero
   struct {
      //
      // If this widget receives a D-Pad input and doesn't return TRUE 
      // from its input handler (or has no input handler), then the 
      // widget-container will attempt to navigate directionally based 
      // on the D-Pad input.
      //
      u8 id_up    : 6;
      u8 id_down  : 6;
      u8 id_left  : 6;
      u8 id_right : 6;
   } navigation;
   bool8 disabled  : 1;
   bool8 focusable : 1;
};

extern void LuVWidget_Destroy(struct LuVWidget*); // invokes "destroy" v-func; clears VTBL
extern bool8 LuVWidget_IsFocusable(const struct LuVWidget*);

struct LuVWidgetContainer {
   struct {
      u8 count;
      struct LuVWidget** list;
   } widgets;
   struct LuVWidget* focused;
};

extern void LuVWidgetContainer_HandleInput(struct LuVWidgetContainer*);
extern struct LuVWidget* LuVWidgetContainer_WidgetByID(struct LuVWidgetContainer*, u8 id);

struct LuVWidget_TypedInValue {
   struct LuVWidget base;
   u8 next_index;
   u8 max_length;
   u8 underscore_sprite_ids[10];
};

#endif