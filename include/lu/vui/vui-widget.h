#ifndef GUARD_LU_VUI_WIDGET_H
#define GUARD_LU_VUI_WIDGET_H

#include "gba/types.h"
#include "lu/vui/vui-types.h"
#include "lu/vui/vui-virtual.h"
#include "lu/c-attr.define.h"

// Frame-handler flags, to be returned by a v-widget's `on_frame` handler.
enum {
   VWIDGET_FRAMEHANDLER_CONSUMED_DPAD = (1 << 0),
};

enum {
   VWIDGET_MIN_SIZE = 1,
};

#define VUI_WIDGET_CLASS_HEADER \
   int __is_vui_widget__[0]
#define VUI_WIDGET_SUBCLASS_HEADER(base_class_name) \
   VUI_WIDGET_CLASS_HEADER; base_class_name base

// Helper macro, used when defining other macros, to fake function names that 
// accept pointers to VUIWidgets or any subclass thereof.
#define VUI_WIDGET_TYPECHECK_AND_CALL(func, widget, ...) \
   _Generic( \
      (widget)->__is_vui_widget__[0], \
      int : func((VUIWidget*)(widget),##__VA_ARGS__) \
   )
// Helper macro, used when defining other macros, to create function-like 
// expressions that act on pointers to VUIWidgets or any subclass thereof.
#define VUI_WIDGET_TYPECHECK_AND_EXEC(widget, expr) \
   _Generic( \
      (widget)->__is_vui_widget__[0], \
      int : (expr) \
   )
   

// ---

struct VUIWidget;

struct VTable_VUIWidget {
   const void* superclass_vtable;
   u8  (* _impl_destroy         )(struct VUIWidget* this);
   u8  (* on_frame              )(struct VUIWidget* this);
   void(* on_focus_change       )(struct VUIWidget* this, bool8 gained, struct VUIWidget*);
   u8  (* on_subgrid_focus_moved)(struct VUIWidget* this);
};
extern const struct VTable_VUIWidget gVTable_VUIWidget;

typedef struct VUIWidget {
   VUI_WIDGET_CLASS_HEADER;
   const struct VTable_VUIWidget* functions;
   bool8 disabled    : 1;
   bool8 focusable   : 1;
   bool8 has_subgrid : 1;
   bool8 context_controls_subgrid_focus : 1;
   //
   // Used to influence directional navigation between widgets.
   // Directional navigation will jump to the nearest widget 
   // in a given direction.
   //
   VUIPos  pos;
   VUISize size;
   //
   // Fields that influence directional navigation if the 
   // `has_subgrid` flag is set.
   //
   VUISize subgrid_size; // subgrid size; 0 = default, per axis
   VUIPos  subgrid_focus;
} VUIWidget;

NON_NULL_PARAMS(1) extern void  VUIWidget_Construct   (VUIWidget* this);
NON_NULL_PARAMS(1) extern void  VUIWidget_Destroy     (VUIWidget* this);
NON_NULL_PARAMS(1) extern bool8 VUIWidget_IsFocusable (const VUIWidget* this);

extern void VUIWidget_SetGridMetrics_(VUIWidget* this, u8 x, u8 y, u8 w, u8 h);
#define VUIWidget_SetGridMetrics(widget, _x, _y, _w, _h) \
   VUI_WIDGET_TYPECHECK_AND_EXEC(widget, (\
      (((VUIWidget*)widget)->pos.x  = (_x)), \
      (((VUIWidget*)widget)->pos.y  = (_y)), \
      (((VUIWidget*)widget)->size.w = (_w) ? (_w) : 1), \
      (((VUIWidget*)widget)->size.h = (_h) ? (_h) : 1), \
      0 \
   ))

#include "lu/c-attr.undef.h"
#endif