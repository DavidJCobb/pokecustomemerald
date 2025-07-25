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

static bool8 _VUIContext_TryNavigate(
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
   if (!dst_widget)
      return FALSE;
   
   if (dst_widget->has_subgrid) {
      VUI_MapBoxToExtents(
         &dst_widget->pos,
         &dst_widget->size,
         (along_axis == VUI_AXIS_X) ? &dm_extent : &dc_extent,
         (along_axis == VUI_AXIS_Y) ? &dm_extent : &dc_extent
      );
      VUIPos cursor = {
         (along_axis == VUI_AXIS_X) ? dst_coord : from_cross_pos,
         (along_axis == VUI_AXIS_Y) ? dst_coord : from_cross_pos
      };
      VUI_MapPosAcrossSizes(&cursor, &dst_widget->size, &dst_widget->subgrid_size);
      //
      // Force destination subgrid-focus position to the subgrid edge that 
      // we're navigating onto:
      //
      if (dst_coord > dm_extent.start) { // coming in from the right/below
         cursor.coords[along_axis] = dst_widget->subgrid_size.bounds[along_axis];
         if (!cursor.coords[along_axis])
            cursor.coords[along_axis] = dst_widget->size.bounds[along_axis];
         --cursor.coords[along_axis];
      } else { // coming in from the left/above
         cursor.coords[along_axis] = 0;
      }
      
      dst_widget->subgrid_focus = cursor;
   }
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