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
      if (dx || dy) {
         _VUIContext_MoveFocus(this, dx, dy);
      }
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

//
// Below are the functions used by directional navigation to find a 
// widget to navigate to, given a target main-axis coordinate and a 
// preferred cross-axis coordinate. One function exists for each 
// choice of main axis (X or Y), and they do essentially the same 
// things but query slightly different fields. Some macros are used 
// here to unify the logic.
//

#define WIDGET_LOOP_START                                                  \
   if (!widget || widget == prior || !VUIWidget_IsFocusable(widget))       \
      continue;                                                            \
   VUIExtent u_extent;                                                     \
   VUIExtent v_extent;                                                     \
   VUI_MapBoxToExtents(&widget->pos, &widget->size, &u_extent, &v_extent);

#define WIDGET_LOOP_MATCH(extent, axis)                    \
   u8 diff = VUI_ExtentDistance(&extent, preferred->axis); \
   if (best_match && diff >= best_dist)                    \
      continue;                                            \
   best_match   = widget;                                  \
   best_dist    = diff;

#define MAP_CURSOR \
   VUI_MapPosAcrossSizes(&cursor, &best_match->size, &best_match->inner_size);

#define COORD_FROM_EDGE(axis, axis_size)              \
   if (d##axis < 0) {                                 \
      cursor.axis = best_match->inner_size.axis_size; \
      if (!cursor.axis)                               \
         cursor.axis = best_match->size.axis_size;    \
      --cursor.axis;                                  \
   } else {                                           \
      cursor.axis = 0;                                \
   }

#define SET_CURSOR \
   best_match->cursor_pos = cursor;

static bool8 _VUIContext_TryNavigateHorizontally(
   VUIContext*      this,
   VUIWidget*       prior,
   u8               adjacent,
   const VUIExtent* y_extent,
   VUIPos*          preferred,
   s8               dx
) {
   VUIWidget* best_match = NULL;
   s8         best_dist  = 0;
   
   vui_context_foreach(this, widget) {
      WIDGET_LOOP_START
      if (!VUI_ExtentOverlaps(&u_extent, adjacent)) // not adjacent
         continue;
      if (!VUI_ExtentsOverlap(&v_extent, y_extent)) // wholly above or below
         continue;
      WIDGET_LOOP_MATCH(v_extent, y)
   }
   if (!best_match)
      return FALSE;
   if (best_match->has_inner_cursor) {
      VUIPos cursor = { adjacent, preferred->y };
      MAP_CURSOR
      COORD_FROM_EDGE(x, w);
      SET_CURSOR
   }
   VUIContext_FocusWidget(this, best_match);
   return TRUE;
}
static bool8 _VUIContext_TryNavigateVertically(
   VUIContext*      this,
   VUIWidget*       prior,
   const VUIExtent* x_extent,
   u8               adjacent,
   VUIPos*          preferred,
   s8               dy
) {
   VUIWidget* best_match = NULL;
   s8         best_dist  = 0;
   
   vui_context_foreach(this, widget) {
      WIDGET_LOOP_START
      if (!VUI_ExtentOverlaps(&v_extent, adjacent)) // not adjacent
         continue;
      if (!VUI_ExtentsOverlap(&u_extent, x_extent)) // wholly to the left or right
         continue;
      WIDGET_LOOP_MATCH(u_extent, x)
   }
   if (!best_match)
      return FALSE;
   if (best_match->has_inner_cursor) {
      VUIPos cursor = { preferred->x, adjacent };
      MAP_CURSOR
      COORD_FROM_EDGE(y, h);
      SET_CURSOR
   }
   VUIContext_FocusWidget(this, best_match);
   return TRUE;
}

#undef WIDGET_LOOP_START
#undef WIDGET_LOOP_MATCH
#undef MAP_CURSOR
#undef COORD_FROM_EDGE
#undef SET_CURSOR

static void _VUIContext_MoveFocus(VUIContext* this, s8 dx, s8 dy) {
   VUIWidget* prior = this->focused;
   AGB_ASSERT(!!prior);
   #if NDEBUG
      vui_context_foreach(this, widget) {
         if (!widget)
            continue;
         AGB_WARNING(widget->size.w >= VWIDGET_MIN_SIZE);
         AGB_WARNING(widget->size.h >= VWIDGET_MIN_SIZE);
      }
   #endif
   
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
      if (_VUIContext_TryNavigateHorizontally(
         this,
         prior,
         ((dx < 0) ? x_extent.start - 1 : x_extent.end),
         &y_extent,
         &preferred,
         dx
      )) {
         return;
      }
   }
   if (dy) {
      if (_VUIContext_TryNavigateVertically(
         this,
         prior,
         &x_extent,
         ((dy < 0) ? y_extent.start - 1 : y_extent.end),
         &preferred,
         dy
      )) {
         return;
      }
   }
   //
   // Nowhere to navigate to in the desired direction. Try wraparound.
   //
   if (dx && this->allow_wraparound_x) {
      AGB_WARNING(this->w > 0);
      if (_VUIContext_TryNavigateHorizontally(
         this,
         prior,
         ((dx > 0) ? 0 : this->w - 1),
         &y_extent,
         &preferred,
         dx
      )) {
         return;
      }
   }
   if (dy && this->allow_wraparound_y) {
      AGB_WARNING(this->h > 0);
      if (_VUIContext_TryNavigateVertically(
         this,
         prior,
         &x_extent,
         ((dy > 0) ? 0 : this->h - 1),
         &preferred,
         dy
      )) {
         return;
      }
   }
}