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

static VUIWidget* _VUIContext_GetNavigationTarget(
   VUIContext*        this,
   VUIWidget*         from_widget,
   const enum VUIAxis along_axis,
   u8                 dst_coord,
   const u8           from_cross_pos
) {
   VUIWidget* dst_widget   = NULL;
   s8         dst_distance = 0; // along cross axis from `from_cross_pos`
   
   VUIExtent fm_extent; // from-widget main-axis  extent
   VUIExtent fc_extent; // from-widget cross-axis extent
   VUI_MapBoxToExtents(
      &from_widget->pos,
      &from_widget->size,
      (along_axis == VUI_AXIS_X) ? &fm_extent : &fc_extent,
      (along_axis == VUI_AXIS_Y) ? &fm_extent : &fc_extent
   );
   
   VUIExtent dm_extent; // widget-being-checked main-axis  extent
   VUIExtent dc_extent; // widget-being-checked cross-axis extent
   
   vui_context_foreach(this, widget) {
      if (!widget || widget == from_widget || !VUIWidget_IsFocusable(widget))
         continue;
      VUI_MapBoxToExtents(
         &widget->pos,
         &widget->size,
         (along_axis == VUI_AXIS_X) ? &dm_extent : &dc_extent,
         (along_axis == VUI_AXIS_Y) ? &dm_extent : &dc_extent
      );
      
      if (!VUI_ExtentOverlaps(&dm_extent, dst_coord)) // not adjacent
         continue;
      if (!VUI_ExtentsOverlap(&dc_extent, &fc_extent))
         continue;
      
      u8 diff = VUI_ExtentDistance(&dc_extent, from_cross_pos);
      if (dst_widget && diff >= dst_distance)
         continue;
      dst_widget   = widget;
      dst_distance = diff;
   }
   
   return dst_widget;
}

static bool8 _VUIContext_TryNavigate(
   VUIContext*        this,
   VUIWidget*         from_widget,
   const enum VUIAxis along_axis,
   u8                 dst_coord,
   const u8           from_cross_pos
) {
   VUIWidget* dst_widget = _VUIContext_GetNavigationTarget(this, from_widget, along_axis, dst_coord, from_cross_pos);
   if (!dst_widget)
      return FALSE;
   
   if (!dst_widget->has_subgrid) {
      VUIContext_FocusWidget(this, dst_widget);
      return TRUE;
   }
   
   //
   // Destination widget has a subgrid. Manage the subgrid focus 
   // appropriately.
   //
   enum {
      PRECISION = 10,
   };
   //
   // Update destination widget's main-axis subgrid focus position.
   //
   if (dst_coord > dst_widget->pos.coords[along_axis]) {
      //
      // Coming in from the right or below.
      //
      u8 pos = dst_widget->subgrid_size.bounds[along_axis];
      if (pos == 0)
         pos = dst_widget->size.bounds[along_axis];
      if (pos > 0)
         --pos;
      dst_widget->subgrid_focus.coords[along_axis] = pos;
   } else {
      //
      // Coming in from the left or above.
      //
      dst_widget->subgrid_focus.coords[along_axis] = 0;
   }
   //
   // If this isn't wraparound navigation from a widget to itself, then 
   // update the cross-axis subgrid focus position as well.
   //
   if (from_widget != dst_widget) {
      u8 cross_axis = !along_axis;
      
      u8 src_size;
      u8 src_pos;
      u8 dst_size;
      {
         dst_size = dst_widget->subgrid_size.bounds[cross_axis];
         if (dst_size == 0)
            dst_size = dst_widget->size.bounds[cross_axis];
         if (dst_size < 1)
            dst_size = 1;
      }
      if (from_widget->has_subgrid) {
         //
         // Subgrid-to-subgrid cursor mapping. The `from_cross_pos` 
         // argument is in context-grid space, but what we really want 
         // here is the source-subgrid-space cross-axis coordinate.
         //
         src_pos  = from_widget->subgrid_focus.coords[cross_axis];
         src_size = from_widget->subgrid_size.bounds[cross_axis];
         if (src_size == 0)
            src_size = from_widget->size.bounds[cross_axis];
      } else {
         //
         // Context-grid-to-subgrid cursor mapping. The `from_cross_pos` 
         // argument is in context-grid space, so we just have to map it 
         // to destination-subgrid space.
         //
         src_pos  = from_cross_pos;
         src_size = dst_widget->size.bounds[cross_axis];
      }
      if (src_size < 1)
         src_size = 1;
      
      if (src_size != dst_size) {
         u8 prior_pos = dst_widget->subgrid_focus.coords[cross_axis];
         u8 after_pos = ((u16)src_pos * PRECISION * dst_size) / src_size / PRECISION;
         //
         // There's an edge-case that'll happen if we just write `after_pos` 
         // into the subgrid-focus position straightaway. Consider this 
         // arrangement of three widgets, the leftmost of which has a subgrid:
         //
         //    +--+--+--+      +--+
         //    |  |  |  |      |  |
         //    +--+--+--+      +--+
         //    |  |  |  |
         //    +--+--+--+      +--+
         //    |  |  |  |      |  |
         //    +--+--+--+      +--+
         //
         // If the player moves the cursor rightward from the top or middle 
         // rows of the subgrid, then they'll end up in the top-right widget. 
         // Here's the edge-case: if they move rightward from the middle row 
         // to the top-right widget, and then move back leftward, then they'd 
         // be snapped to the top row instead of being returned to the middle 
         // row.
         //
         // The way to avoid this is simple: perform the mapping in reverse. 
         // At the time that we navigate from the top-right widget back to 
         // the subgridded widget, the latter still has its subgrid focus 
         // position in its middle row. So, we see if the middle row would 
         // map to our source widget, and if so, we opt *not* to move the 
         // subgrid focus cross-axis position.
         //
         u8 reversed  = ((u16)prior_pos * PRECISION * src_size) / dst_size / PRECISION;
         if (reversed != src_pos) {
            dst_widget->subgrid_focus.coords[cross_axis] = after_pos;
         }
      }
   }
   //
   // Finally, set focus.
   //
   VUIContext_FocusWidget(this, dst_widget);
   return TRUE;
}
//
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
   
   //
   // If the currently focused widget allows the context to control its 
   // subgrid focus, then try moving the subgrid focus.
   //
   if (prior->has_subgrid && prior->context_controls_subgrid_focus) {
      bool8 moved = FALSE;
      if (dx) {
         if (
            (dx < 0 && prior->subgrid_focus.x > 0)
         || (dx > 0 && prior->subgrid_focus.x < prior->subgrid_size.w - 1)
         ) {
            prior->subgrid_focus.x += dx;
            moved = TRUE;
         }
      }
      if (dy) {
         if (
            (dy < 0 && prior->subgrid_focus.y > 0)
         || (dy > 0 && prior->subgrid_focus.y < prior->subgrid_size.h - 1)
         ) {
            prior->subgrid_focus.y += dy;
            moved = TRUE;
         }
      }
      if (moved) {
         if (v_can_invoke(prior, on_subgrid_focus_moved))
            v_invoke(prior, on_subgrid_focus_moved)();
         return;
      }
   }
   
   VUIExtent x_extent;
   VUIExtent y_extent;
   VUIPos    preferred;
   
   VUI_MapBoxToExtents(&prior->pos, &prior->size, &x_extent, &y_extent);
   
   if (prior->has_subgrid) {
      preferred = prior->subgrid_focus;
      VUI_MapPosAcrossSizes(&preferred, &prior->subgrid_size, &prior->size);
   } else {
      preferred = prior->pos;
      preferred.x += (prior->size.w / 2);
      preferred.y += (prior->size.h / 2);
   }
   
   VUIWidget* best_match = NULL;
   s8         best_dist  = 0;
   
   if (dx) {
      s16 beyond = VUI_PointBeyondExtent(&x_extent, dx);
      if (beyond >= 0 && _VUIContext_TryNavigate(this, prior, VUI_AXIS_X, beyond, preferred.y)) {
         return;
      }
   }
   if (dy) {
      s16 beyond = VUI_PointBeyondExtent(&y_extent, dy);
      if (beyond >= 0 && _VUIContext_TryNavigate(this, prior, VUI_AXIS_Y, beyond, preferred.x)) {
         return;
      }
   }
   //
   // Nowhere to navigate to in the desired direction. Try wraparound.
   //
   if (dx && this->allow_wraparound_x) {
      AGB_WARNING(this->w > 0);
      s16 beyond = (dx > 0) ? 0 : this->w - 1;
      if (beyond >= 0 && _VUIContext_TryNavigate(this, prior, VUI_AXIS_X, beyond, preferred.y)) {
         return;
      }
   }
   if (dy && this->allow_wraparound_y) {
      AGB_WARNING(this->h > 0);
      s16 beyond = (dy > 0) ? 0 : this->h - 1;
      if (beyond >= 0 && _VUIContext_TryNavigate(this, prior, VUI_AXIS_Y, beyond, preferred.x)) {
         return;
      }
   }
   //
   // Try wraparound within the subgrid.
   //
   if (prior->has_subgrid && prior->context_controls_subgrid_focus) {
      bool8 moved = FALSE;
      if (dx && this->allow_wraparound_x) {
         if (dx < 0) {
            prior->subgrid_focus.x = prior->subgrid_size.w - 1;
         } else {
            prior->subgrid_focus.x = 0;
         }
         moved = TRUE;
      }
      if (dy && this->allow_wraparound_y) {
         if (dy < 0) {
            prior->subgrid_focus.y = prior->subgrid_size.h - 1;
         } else {
            prior->subgrid_focus.y = 0;
         }
         moved = TRUE;
      }
      if (moved) {
         if (v_can_invoke(prior, on_subgrid_focus_moved))
            v_invoke(prior, on_subgrid_focus_moved)();
      }
   }
}