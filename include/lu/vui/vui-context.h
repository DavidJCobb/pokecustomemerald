#ifndef GUARD_LU_VUI_CONTEXT_H
#define GUARD_LU_VUI_CONTEXT_H

#include "lu/c-attr.define.h"
#include "gba/types.h"

struct VUIWidget;

typedef struct VUIContext {
   struct {
      struct VUIWidget** list;
      u8                 size;
   } widgets;
   struct VUIWidget* focused;
   
   bool8 allow_wraparound_x : 1;
   bool8 allow_wraparound_y : 1;
   u8 w;
   u8 h;
} VUIContext;

NON_NULL_PARAMS(1)   extern void VUIContext_HandleInput(VUIContext* this);
NON_NULL_PARAMS(1,2) extern void VUIContext_FocusWidget(VUIContext* this, struct VUIWidget*);

/*
   Usage:
   
   vui_context_foreach(context, new_variable_name) {
      // code using `new_variable_name`
   }
*/
#define vui_context_foreach(context, varname) vui_context_foreach_impl(context, varname, __LINE__)

#define vui_context_foreach_impl(context, varname, i) \
   u8 __i_##i##__ = 0; \
   for( \
      struct VUIWidget* varname = (context)->widgets.list[0]; \
      __i_##i##__ < (context)->widgets.size && (varname = (context)->widgets.list[__i_##i##__], 1); \
      ++__i_##i##__)

#include "lu/c-attr.undef.h"
#endif