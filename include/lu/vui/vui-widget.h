#ifndef GUARD_LU_VUI_WIDGET_H
#define GUARD_LU_VUI_WIDGET_H

#include "gba/types.h"
#include "lu/vui/vui-types.h"
#include "lu/vui/vui-virtual.h"

// Frame-handler flags, to be returned by a v-widget's `on_frame` handler.
enum {
   VWIDGET_FRAMEHANDLER_CONSUMED_DPAD = (1 << 0),
};

enum {
   VWIDGET_MIN_SIZE = 1,
};

// ---

struct VUIWidget;

struct VTable_VUIWidget {
   const void* superclass_vtable;
   u8  (* _impl_destroy  )(struct VUIWidget* this);
   u8  (* on_frame       )(struct VUIWidget* this);
   void(* on_focus_change)(struct VUIWidget* this, bool8 gained, struct VUIWidget*);
};
extern const struct VTable_VUIWidget gVTable_VUIWidget;

typedef struct VUIWidget {
   const struct VTable_VUIWidget* functions;
   bool8 disabled    : 1;
   bool8 focusable   : 1;
   bool8 has_subgrid : 1;
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

extern void  VUIWidget_Construct   (VUIWidget* this);
extern void  VUIWidget_Destroy     (VUIWidget* this);
extern bool8 VUIWidget_IsFocusable (const VUIWidget* this);

#endif