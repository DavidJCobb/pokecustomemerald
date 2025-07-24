#include "lu/vui/tile-button.h"
#include "gba/isagbprint.h"
#include "global.h" // JOY_NEW and friends
#include "bg.h"
#include "main.h"
#include "menu.h" // AddTextPrinterParameterized3
#include "window.h"

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
   
   this->bg        = params->bg;
   this->callbacks = params->callbacks;
   this->labels    = params->labels;
   this->palette   = params->palette;
   this->tile_area = params->tile_area;
   this->tile_ids.normal  = params->tile_ids.normal;
   this->tile_ids.focused = params->tile_ids.focused;
   
   if (params->tile_area.size.w > 0 && params->tile_area.size.h > 0) {
      const struct WindowTemplate tmpl = {
         .bg          = params->bg,
         .tilemapLeft = params->tile_area.pos.x,
         .tilemapTop  = params->tile_area.pos.y,
         .width       = params->tile_area.size.w,
         .height      = params->tile_area.size.h,
         .paletteNum  = params->palette,
         .baseBlock   = params->tile_ids.first_window_tile_id
      };
      u8 window_id = AddWindow(&tmpl);
      AGB_ASSERT(window_id != WINDOW_NONE);
      this->window_id = window_id;
      
      PutWindowTilemap(window_id);
   }
   
   Repaint(this, FALSE);
}

static u8 VFunc_DestroyImpl(VUIWidget* widget) {
   VUITileButton* this = (VUITileButton*)widget;
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
   Repaint(this, gained);
}

// Assumes 4bpp source and destination.
static void FlipWindowTile(
   u8    window_id,
   u8    tile_x,
   u8    tile_y,
   bool8 flip_h,
   bool8 flip_v
) {
   enum {
      BYTES_PER_ROW = 4,
   };
   #define XOR_SWAP(_a, _b) do { _a = _b ^ _a; _b = _a ^ _b; _a = _b ^ _a; } while (0)
   
   if (!flip_h && !flip_v)
      return;
   u8* buffer = gWindows[window_id].tileData;
   if (!buffer)
      return;
   u8  tile_index  = tile_y * gWindows[window_id].window.width + tile_x;
   u8* tile_buffer = &buffer[tile_index * TILE_SIZE_4BPP];
   
   if (flip_h) {
      //
      // Swap nybbles within each byte.
      //
      for(u8 i = 0; i < TILE_SIZE_4BPP; ++i) {
         u8 byte = tile_buffer[i];
         byte = ((byte & 0xF) << 4) | (byte >> 4);
         tile_buffer[i] = byte;
      }
      //
      // Swap byte order for each row.
      //
      for(u8 y = 0; y < TILE_HEIGHT; ++y) {
         for(u8 hx = 0; hx < BYTES_PER_ROW; ++hx) {
            u8* row = &tile_buffer[y * BYTES_PER_ROW + hx];
            XOR_SWAP(row[0], row[3]);
            XOR_SWAP(row[1], row[2]);
         }
      }
   }
   if (flip_v) {
      //
      // Swap rows.
      //
      for(u8 y = 0; y < TILE_HEIGHT; ++y) {
         u8* row_a = &tile_buffer[y * BYTES_PER_ROW];
         u8* row_b = &tile_buffer[(TILE_HEIGHT - y - 1) * BYTES_PER_ROW];
         for(u8 hx = 0; hx < BYTES_PER_ROW; ++hx) {
            XOR_SWAP(row_a[hx], row_b[hx]);
         }
      }
   }
   
   #undef XOR_SWAP
}

static void Repaint(VUITileButton* this, bool8 is_focused) {
   const u16 TILE_FLIP_H = 1 << 10;
   const u16 TILE_FLIP_V = 1 << 11;
   
   struct VUITileButton_TileIDs* tile_ids = is_focused ? &this->tile_ids.focused : &this->tile_ids.normal;
   
   u8 w = this->tile_area.size.w;
   u8 h = this->tile_area.size.h;
   if (w < 1)
      w = 1;
   if (h < 1)
      h = 1;
   
   u8 x1 = this->tile_area.pos.x;
   u8 x2 = x1 + w - 1;
   u8 y1 = this->tile_area.pos.y;
   u8 y2 = y1 + h - 1;
   
   // corners
   BlitBitmapToWindow(
      this->window_id,
      &this->tile_data[base]
   );
   
   // corners
   for(u8 i = 0; i < 4; ++i) {
      bool8 flip_h = (i % 2) ? 1 : 0;
      bool8 flip_v = (i / 2) ? 1 : 0;
      
      u16 tile_id = tile_ids->corner;
      if (flip_h)
         tile_id |= TILE_FLIP_H;
      if (flip_v)
         tile_id |= TILE_FLIP_V;
      
      FillBgTilemapBufferRect(
         this->bg,
         tile_id,
         flip_h ? x2 : x1,
         flip_v ? y2 : y1,
         1,
         1,
         this->palette
      );
   }
   
   // h-edges
   if (h > 2) {
      u16 tile_id = tile_ids->edge_h;
      FillBgTilemapBufferRect(this->bg, tile_id, x1, y1 + 1, 1, h - 2, this->palette);
      tile_id |= TILE_FLIP_H;
      FillBgTilemapBufferRect(this->bg, tile_id, x2, y1 + 1, 1, h - 2, this->palette);
   }
   
   // v-edges
   if (w > 2) {
      u16 tile_id = tile_ids->edge_v;
      FillBgTilemapBufferRect(this->bg, tile_id, x1 + 1, y1, w - 2, 1, this->palette);
      tile_id |= TILE_FLIP_V;
      FillBgTilemapBufferRect(this->bg, tile_id, x1 + 1, y2, w - 2, 1, this->palette);
   }
   
   if (this->window_id != WINDOW_NONE) {
      FillWindowPixelBuffer(this->window_id, PIXEL_FILL(this->labels.colors.back));
      
      u8 avail = (w - 2) * TILE_WIDTH;
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
   } else if (w > 2 && h > 2) {
      FillBgTilemapBufferRect(this->bg, tile_ids->fill, x1 + 1, y1 + 1, w - 2, h - 2, this->palette);
   }
}