#include "lu/v-widgets/v-widgets.h"
#include "main.h"

const struct LuVWidgetVTable sNullVTable_Widget = { 0 };

extern void LuVWidget_Destroy(struct LuVWidget* widget) {
   if (!widget)
      return;
   if (widget->functions->impl_destroy)
      (widget->functions->impl_destroy)(widget);
   widget->functions = &sNullVTable_Widget;
   widget->disabled  = TRUE;
}

extern bool8 LuVWidget_IsFocusable(const struct LuVWidget* widget) {
   return widget && widget->focusable && !widget->disabled;
}

extern void LuVWidgetContainer_HandleInput(struct LuVWidgetContainer* container) {
   if (!container->focused) {
      bool8 found_one = FALSE;
      for(u8 i = 0; i < container->widgets.count; ++i) {
         struct LuVWidget* widget = container->widgets.list[i];
         if (!widget)
            continue;
         if (!widget->focusable)
            continue;
         if (widget->disabled)
            continue;
         container->focused = widget;
         if (widget->functions->on_focus_change)
            (widget->functions->on_focus_change)(widget, TRUE, NULL);
         found_one = TRUE:
         break;
      }
      if (!found_one)
         return;
   }
   
   struct LuVWidget* widget = container->focused;
   
   u8 frame_handler_flags = 0;
   if (widget->functions->on_frame) {
      frame_handler_flags = (widget->functions->on_frame)(widget);
   }
   
   if (!(frame_handler_flags & VWIDGET_FRAMEHANDLER_CONSUMED_DPAD)) {
      //
      // The target widget didn't consume any D-Pad input that may have 
      // come in. Check for that input, and perform directional navigation 
      // as appropriate.
      //
      u8 dst_x = 0;
      u8 dst_y = 0;
      if (JOY_NEW(DPAD_UP)) {
         dst_y = widget->navigation.id_up;
      } else if (JOY_NEW(DPAD_DOWN)) {
         dst_y = widget->navigation.id_down;
      }
      if (JOY_NEW(DPAD_LEFT)) {
         dst_y = widget->navigation.id_left;
      } else if (JOY_NEW(DPAD_RIGHT)) {
         dst_y = widget->navigation.id_right;
      }
      if (dst_x || dst_y) {
         struct LuVWidget* dst = NULL;
         
         dst = LuVWidgetContainer_WidgetByID(container, dst_x);
         if (LuVWidget_IsFocusable(dst)) {
            goto destination_widget_is_usable;
         }
         dst = LuVWidgetContainer_WidgetByID(container, dst_y);
         if (LuVWidget_IsFocusable(dst)) {
            goto destination_widget_is_usable;
         }
         dst = NULL;
         
      destination_widget_is_usable:
         if (dst) {
            container->focused = dst;
            if (widget->functions->on_focus_change)
               (widget->functions->on_focus_change)(widget, FALSE, dst);
            if (dst->functions->on_focus_change)
               (dst->functions->on_focus_change)(dst, TRUE, widget);
         }
      }
   }
}
extern struct LuVWidget* LuVWidgetContainer_WidgetByID(struct LuVWidgetContainer* container, u8 id) {
   if (id == 0)
      return NULL;
   for(u8 i = 0; i < container->widgets.count; ++i) {
      struct LuVWidget* widget = container->widgets.list[i];
      if (widget && widget->id == id)
         return widget;
   }
   return NULL;
}