#ifndef GUARD_LU_V_WIDGETS_H
#define GUARD_LU_V_WIDGETS_H

#include "gba/types.h"
#include "lu/v-widgets/vui-types.h"

// ---

extern void _VUI_Invoke_Stub(...);

#define v_invoke(this, func) ( ((this)->functions->func ? (this)->functions->func : _VUI_Invoke_Stub)((this) v_invoke_impl_close

#define v_invoke_impl_close(...) ,##__VA_ARGS__))

// Frame-handler flags, to be returned by a v-widget's `on_frame` handler.
enum {
   VWIDGET_FRAMEHANDLER_CONSUMED_DPAD,
};

enum {
   VWIDGET_MIN_SIZE = 1,
};

// ---

struct VUIWidget;

struct VTable_VUIWidget {
   void* superclass_vtable;
   u8  (* _impl_destroy  )(struct VUIWidget* this);
   u8  (* on_frame       )(struct VUIWidget* this);
   void(* on_focus_change)(struct VUIWidget* this, bool8 gained, struct VUIWidget*);
};

typedef struct VUIWidget {
   struct VTable_VUIWidget* functions;
   bool8 disabled         : 1;
   bool8 focusable        : 1;
   bool8 has_inner_cursor : 1;
   //
   // Used to influence directional navigation between widgets.
   // Directional navigation will jump to the nearest widget 
   // in a given direction.
   //
   VUIPos  pos;
   VUISize size;
   //
   // Fields that influence directional navigation if the 
   // `has_inner_cursor` flag is set.
   //
   VUISize inner_size; // subgrid size; 0 = default, per axis
   VUIPos  cursor_pos;
} VUIWidget;

extern void  VUIWidget_Construct   (VUIWidget* this);
extern void  VUIWidget_Destroy     (VUIWidget* this);
extern bool8 VUIWidget_IsFocusable (const VUIWidget* this);

// ---

typedef struct VUIContext {
   struct {
      VUIWidget** list;
      u8          size;
   } widgets
   VUIWidget* focused;
} VUIContext;

extern void VUIContext_HandleInput(VUIContext* this);

/*
   Usage:
   
   vui_context_foreach(context, new_variable_name) {
      // code using `new_variable_name`
   }
*/
#define vui_context_foreach(context, varname) \
   for( \
      u8 __i__ = 0, VUIWidget* varname = (context)->widgets.list[0]; \
      __i__ < (context)->widgets.size && (varname = (context)->widgets.list[__i__], 1); \
      ++__i__)

#endif