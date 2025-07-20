#include "lu/vui/vui-context.h"
#include "lu/vui/vui-widget.h"
#include "lu/vui/vui-types.h"
#include "global.h"
#include "main.h"

static void _VUIContext_FocusAnyWidget(VUIContext* this);
static void _VUIContext_MoveFocus(VUIContext* this, s8 dx, s8 dy);

extern void VUIContext_HandleInput(VUIContext* this) {
   VUIWidget* widget = this->focused;
   if (!widget) {
      _VUIContext_FocusAnyWidget(this);
      widget = this->focused;
      if (!widget)
         return;
   }
   
   u8 frame_handler_flags = 0;
   if (v_can_invoke(widget, on_frame))
      frame_handler_flags = v_invoke(widget, on_frame)();
   if ((frame_handler_flags & VWIDGET_FRAMEHANDLER_CONSUMED_DPAD) == 0) {
      s8 dx = 0;
      s8 dy = 0;
      if (JOY_NEW(DPAD_UP)) {
         dy = -1;
      } else if (JOY_NEW(DPAD_DOWN)) {
         dy = 1;
      }
      if (JOY_NEW(DPAD_LEFT)) {
         dx = -1;
      } else if (JOY_NEW(DPAD_RIGHT)) {
         dx = 1;
      }
      _VUIContext_MoveFocus(this, dx, dy);
   }
}
extern void VUIContext_FocusWidget(VUIContext* this, VUIWidget* widget) {
   VUIWidget* prior = this->focused;
   if (prior == widget)
      return;
   this->focused = widget;
   if (v_can_invoke(prior, on_focus_change))
      v_invoke(prior, on_focus_change)(FALSE, widget);
   if (v_can_invoke(widget, on_focus_change))
      v_invoke(widget, on_focus_change)(TRUE, prior);
}

static void _VUIContext_FocusAnyWidget(VUIContext* this) {
   vui_context_foreach(this, widget) {
      if (widget && VUIWidget_IsFocusable(widget)) {
         VUIContext_FocusWidget(this, widget);
         return;
      }
   }
}

static void _VUIContext_SetFocusAndCursor(
   VUIContext* this,
   VUIWidget*  widget,
   VUIPos*     cursor
) {
   if (widget->has_inner_cursor) {
      VUI_MapPosAcrossSizes(
         cursor,
         &widget->size,
         &widget->inner_size
      );
      widget->cursor_pos = *cursor;
   }
   VUIContext_FocusWidget(this, widget);
}

static void _VUIContext_MoveFocus(VUIContext* this, s8 dx, s8 dy) {
   VUIWidget* prior = this->focused;
   AGB_ASSERT(!!prior);
   AGB_WARNING(prior->size.w >= VWIDGET_MIN_SIZE);
   AGB_WARNING(prior->size.h >= VWIDGET_MIN_SIZE);
   
   VUIExtent x_extent;
   VUIExtent y_extent;
   VUIPos    preferred;
   
   VUI_MapBoxToExtents(&prior->pos, &prior->size, &x_extent, &y_extent);
   
   if (prior->has_inner_cursor) {
      preferred = prior->cursor_pos;
      VUI_MapPosAcrossSizes(&preferred, &prior->inner_size, &prior->size);
   } else {
      preferred = prior->pos;
      preferred.x += (prior->size.w / 2);
      preferred.y += (prior->size.h / 2);
   }
   
   VUIWidget* best_match = NULL;
   s8         best_dist  = 0;
   
   if (dx) {
      vui_context_foreach(this, widget) {
         if (!widget || widget == prior || !VUIWidget_IsFocusable(widget))
            continue;
         AGB_WARNING(widget->size.w >= VWIDGET_MIN_SIZE);
         AGB_WARNING(widget->size.h >= VWIDGET_MIN_SIZE);
         VUIExtent u_extent;
         VUIExtent v_extent;
         VUI_MapBoxToExtents(&widget->pos, &widget->size, &u_extent, &v_extent);
         
         if (!VUI_ExtentsOverlap(&v_extent, &y_extent)) // wholly above or below
            continue;
         
         if (dx > 0) { // searching rightward
            if (u_extent.end <= x_extent.start) // is wholly to the left
               continue;
         } else { // searching leftward
            if (u_extent.start >= x_extent.end) // is wholly to the right
               continue;
         }
         
         u8 diff = VUI_ExtentDistance(&v_extent, preferred.y);
         if (best_match && diff >= best_dist)
            continue;
         best_match = widget;
         best_dist  = diff;
      }
      if (best_match) {
DebugPrintf("[VUIContext] Transferring focus horizontally to other another control...");
         preferred.x = best_match->pos.x;
         if (dx < 0)
            preferred.x += best_match->size.w;
         _VUIContext_SetFocusAndCursor(this, best_match, &preferred);
         return;
      }
   }
   if (!best_match && dy) {
      vui_context_foreach(this, widget) {
         if (!widget || widget == prior || !VUIWidget_IsFocusable(widget))
            continue;
         AGB_WARNING(widget->size.w >= VWIDGET_MIN_SIZE);
         AGB_WARNING(widget->size.h >= VWIDGET_MIN_SIZE);
         VUIExtent u_extent;
         VUIExtent v_extent;
         VUI_MapBoxToExtents(&widget->pos, &widget->size, &u_extent, &v_extent);
         
         if (!VUI_ExtentsOverlap(&u_extent, &x_extent)) // wholly to the left or right
            continue;
         
         if (dy > 0) { // searching downward
            if (v_extent.end <= y_extent.start) // is wholly above
               continue;
         } else { // searching upward
            if (v_extent.start >= y_extent.end) // is wholly below
               continue;
         }
         
         u8 diff = VUI_ExtentDistance(&u_extent, preferred.x);
         if (best_match && diff >= best_dist)
            continue;
         best_match = widget;
         best_dist  = diff;
      }
      if (best_match) {
DebugPrintf("[VUIContext] Transferring focus vertically to other another control...");
         preferred.y = best_match->pos.y;
         if (dx < 0)
            preferred.y += best_match->size.h;
         _VUIContext_SetFocusAndCursor(this, best_match, &preferred);
         return;
      }
   }
}