#include "lu/vui/tile-button.h"
#include "gba/defines.h"
#include "gba/isagbprint.h"
#include "global.h" // JOY_NEW and friends
#include "bg.h"
#include "main.h"
#include "menu.h" // AddTextPrinterParameterized3
#include "window.h"
#include "lu/gfxutils.h"

static u8 VFunc_DestroyImpl(VUIWidget*);
static u8 VFunc_OnFrame(VUIWidget*);
static void VFunc_OnFocusChange(VUIWidget*, bool8, VUIWidget*);
//
static const struct VTable_VUIWidget sVTable = {
   &gVTable_VUIWidget,
   VFunc_DestroyImpl,
   VFunc_OnFrame,
   VFunc_OnFocusChange,
};

static void Repaint(VUITileButton*, bool8 is_focused);

extern void VUITileButton_Construct(VUITileButton* this, const VUITileButton_InitParams* params) {
   this->window_id = WINDOW_NONE;
   
   VUIWidget_Construct(&this->base);
   this->base.functions = &sVTable;
   this->base.focusable = TRUE;
   VUIWidget_SetGridMetrics(this, params->grid.pos.x, params->grid.pos.y, params->grid.size.w, params->grid.size.h);
   
   this->callbacks = params->callbacks;
   this->labels    = params->labels;
   this->tiles     = params->tiles;
   
   if (params->tiles.size.w > 0 && params->tiles.size.h > 0) {
      const struct WindowTemplate tmpl = {
         .bg          = params->tiles.bg,
         .tilemapLeft = params->screen_pos.x,
         .tilemapTop  = params->screen_pos.y,
         .width       = params->tiles.size.w,
         .height      = params->tiles.size.h,
         .paletteNum  = params->tiles.palette,
         .baseBlock   = params->first_window_tile_id
      };
      u8 window_id = AddWindow(&tmpl);
      AGB_ASSERT(window_id != WINDOW_NONE);
      this->window_id = window_id;
      
      PutWindowTilemap(window_id);
   }
}

static u8 VFunc_DestroyImpl(VUIWidget* widget) {
   VUITileButton* this = (VUITileButton*)widget;
   if (this->window_id != WINDOW_NONE) {
      RemoveWindow(this->window_id);
      this->window_id = WINDOW_NONE;
   }
}
static u8 VFunc_OnFrame(VUIWidget* widget) {
   VUITileButton* this = (VUITileButton*)widget;
   if (JOY_NEW(A_BUTTON)) {
      if (this->callbacks.on_press)
         (this->callbacks.on_press)();
      return VWIDGET_FRAMEHANDLER_CONSUMED_DPAD;
   }
   return 0;
}
static void VFunc_OnFocusChange(VUIWidget* widget, bool8 gained, VUIWidget*) {
   VUITileButton* this = (VUITileButton*)widget;
   VUITileButton_Repaint(this, gained);
}

extern void VUITileButton_Repaint(VUITileButton* this, bool8 is_focused) {
   enum {
      TILEIDX_CORNER  = 0,
      TILEIDX_EDGE_TB = 1,
      TILEIDX_EDGE_LR = 2,
   };
   if (!this->tiles.data)
      return;
   if (this->window_id == WINDOW_NONE)
      return;
   const u8* tile_data = &this->tiles.data[(is_focused ? 3 : 0) * TILE_SIZE_4BPP];
   
   FillWindowPixelBuffer(this->window_id, PIXEL_FILL(0));
   
   u8 w = this->tiles.size.w;
   u8 h = this->tiles.size.h;
   if (w < 1)
      w = 1;
   if (h < 1)
      h = 1;
   
   const u8 px_r = (w - 1) * TILE_WIDTH; // right
   const u8 px_b = (h - 1) * TILE_WIDTH; // bottom
   
   // corners
   {
      const u8* tile = &tile_data[TILEIDX_CORNER * TILE_SIZE_4BPP];
      BlitBitmapToWindow(this->window_id, tile, 0,    0,    TILE_WIDTH, TILE_HEIGHT);
      BlitBitmapToWindow(this->window_id, tile, px_r, 0,    TILE_WIDTH, TILE_HEIGHT);
      BlitBitmapToWindow(this->window_id, tile, 0,    px_b, TILE_WIDTH, TILE_HEIGHT);
      BlitBitmapToWindow(this->window_id, tile, px_r, px_b, TILE_WIDTH, TILE_HEIGHT);
      
      // in-place pixel flips:
      FlipWindowTile(this->window_id, w - 1, 0,     TRUE,  FALSE);
      FlipWindowTile(this->window_id, 0,     h - 1, FALSE, TRUE);
      FlipWindowTile(this->window_id, w - 1, h - 1, TRUE,  TRUE);
   }
   // horizontal edges:
   if (w > 2) {
      const u8* tile = &tile_data[TILEIDX_EDGE_TB * TILE_SIZE_4BPP];
      for(u8 i = 1; i < w - 1; ++i) {
         BlitBitmapToWindow(this->window_id, tile, i * TILE_WIDTH, 0, TILE_WIDTH, TILE_HEIGHT);
      }
      
      // Blit a single tile onto the bottom edge, and flip it:
      BlitBitmapToWindow(this->window_id, tile, TILE_WIDTH, px_b, TILE_WIDTH, TILE_HEIGHT);
      FlipWindowTile(this->window_id, 1, h - 1, FALSE, TRUE);
      if (w > 3) {
         // Now blit from that one flipped tile to the rest of the bottom edge. Now we don't 
         // have to flip the whole edge as a separate step!
         tile = GetWindowTilePtr(this->window_id, 1, h - 1);
         for(u8 i = 2; i < w - 1; ++i) {
            BlitBitmapToWindow(
               this->window_id,
               tile,
               i * TILE_WIDTH,
               px_b,
               TILE_WIDTH, TILE_HEIGHT
            );
         }
      }
   }
   // vertical edges:
   if (h > 2) {
      const u8* tile = &tile_data[TILEIDX_EDGE_LR * TILE_SIZE_4BPP];
      for(u8 i = 1; i < h - 1; ++i) {
         BlitBitmapToWindow(this->window_id, tile, 0, i * TILE_WIDTH, TILE_WIDTH, TILE_HEIGHT);
      }
      
      // Blit a single tile onto the bottom edge, and flip it:
      BlitBitmapToWindow(this->window_id, tile, px_r, TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
      FlipWindowTile(this->window_id, w - 1, 1, TRUE, FALSE);
      if (h > 3) {
         tile = GetWindowTilePtr(this->window_id, w - 1, 1);
         for(u8 i = 2; i < h - 1; ++i) {
            BlitBitmapToWindow(
               this->window_id,
               tile,
               px_r,
               i * TILE_HEIGHT,
               TILE_WIDTH, TILE_HEIGHT
            );
         }
      }
   }
   
   if (w > 2 && h > 2) {
      FillWindowPixelRect(
         this->window_id,
         PIXEL_FILL(this->labels.colors.back),
         TILE_WIDTH,
         TILE_HEIGHT,
         (w - 2) * TILE_WIDTH,
         (h - 2) * TILE_HEIGHT
      );
   }
   
   u8 avail = w * TILE_WIDTH;
   if (this->labels.text != NULL) {
      u16 width = GetStringWidth(FONT_NORMAL, this->labels.text, 0);
      u8  px_x;
      if (width >= avail) {
         px_x = 0;
      } else {
         px_x = (avail - width) / 2;
      }
      AddTextPrinterParameterized3(
         this->window_id,
         FONT_NORMAL,
         px_x,
         this->labels.y_text,
         this->labels.colors.list,
         TEXT_SKIP_DRAW,
         this->labels.text
      );
   }
   if (this->labels.button != NULL) {
      u16 width = GetStringWidth(FONT_NORMAL, this->labels.button, 0);
      u8  px_x;
      if (width >= avail) {
         px_x = 0;
      } else {
         px_x = (avail - width) / 2;
      }
      AddTextPrinterParameterized3(
         this->window_id,
         FONT_NORMAL,
         px_x,
         this->labels.y_button,
         this->labels.colors.list,
         TEXT_SKIP_DRAW,
         this->labels.button
      );
   }
   CopyWindowToVram(this->window_id, COPYWIN_FULL);
}