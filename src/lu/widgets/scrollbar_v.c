#include "lu/widgets/scrollbar_v.h"
#include <string.h>
#include "gba/defines.h" // TILE_HEIGHT
#include "bg.h"
#include "malloc.h"

#define SCROLLBAR_THICKNESS_PX 3

#define SCROLLBAR_INSET_LEFT_PX (TILE_WIDTH - SCROLLBAR_THICKNESS_PX) / 2
#define SCROLLBAR_INSET_RIGHT_PX (SCROLLBAR_INSET_LEFT_PX + SCROLLBAR_THICKNESS_PX)

static void FillTiles(u8* tile_data, u8 tile_count, u8 pal_index) {
   pal_index  = pal_index & 0x0F;
   pal_index |= pal_index << 4;
   memset(tile_data, pal_index, 0x20 * tile_count);
}

extern void InitScrollbarV(struct LuScrollbar* widget) {
   if (!widget->graphics.tile_buffer) {
      widget->graphics.tile_buffer = AllocZeroed((u16)32 * SCROLLBAR_TILE_COUNT);
   }
   FillTiles(widget->graphics.tile_buffer, widget->graphics.colors.background, SCROLLBAR_TILE_COUNT);
   
   // Load drawn tiles to VRAM:
   LoadBgTiles(
      widget->graphics.bg_layer,
      widget->graphics.tile_buffer,
      0x20 * 1 * SCROLLBAR_TILE_COUNT, // size of tile graphic * width * height
      widget->graphics.first_tile_id
   );
   
   // Update tilemap for where our scrollbar has been placed, to use our new tiles.
   FillBgTilemapBufferRect(
      widget->graphics.bg_layer,
      widget->graphics.first_tile_id,
      widget->graphics.pos.x,
      widget->graphics.pos.y,
      1,
      widget->graphics.length,
      widget->graphics.palette_id
   );
}

static u8* GetNthTileData(struct LuScrollbar* widget, u8 tile_index) {
   return (u8*)widget->graphics.tile_buffer + (tile_index * 0x20);
}

static void PaintFullTile(u8* tile_data, u8 pal_index) {
   u8 x; // pixels
   u8 y; // pixels
   for(y = 0; y < TILE_HEIGHT; ++y) {
      for(x = SCROLLBAR_INSET_LEFT_PX; x < SCROLLBAR_INSET_RIGHT_PX; ++x) {
         u8* dst_byte = &tile_data[(y * 4) + (x / 2)];
         
         if (!(x & 1)) { // LSBs = left; MSBs = right
            *dst_byte &= 0xF0;
            *dst_byte |= pal_index;
         } else {
            *dst_byte &= 0x0F;
            *dst_byte |= pal_index << 4;
         }
      }
   }
}
static void PaintEdgeTile(
   u8* tile_data,
   u8  pal_index_a,
   u8  pal_index_b,
   u8  color_switches_at // pixel offset within tile
) {
   u8 x; // pixels
   u8 y; // pixels
   for(y = 0; y < TILE_HEIGHT; ++y) {
      for(x = SCROLLBAR_INSET_LEFT_PX; x < SCROLLBAR_INSET_RIGHT_PX; ++x) {
         u8* dst_byte = &tile_data[(y * 4) + (x / 2)];
         
         u8 pal_index = pal_index_a;
         if (y > color_switches_at) {
            pal_index = pal_index_b;
         }
         
         if (!(x & 1)) { // LSBs = left; MSBs = right
            *dst_byte &= 0xF0;
            *dst_byte |= pal_index;
         } else {
            *dst_byte &= 0x0F;
            *dst_byte |= pal_index << 4;
         }
      }
   }
}
static void PaintDoubleEdgeTile(
   u8* tile_data,
   u8  pal_index_a,
   u8  pal_index_b,
   u8  pal_index_c,
   u8  switch_ab, // pixel offset within tile
   u8  switch_bc  // pixel offset within tile
) {
   u8 x; // pixels
   u8 y; // pixels
   for(y = 0; y < TILE_HEIGHT; ++y) {
      for(x = SCROLLBAR_INSET_LEFT_PX; x < SCROLLBAR_INSET_RIGHT_PX; ++x) {
         u8* dst_byte = &tile_data[(y * 4) + (x / 2)];
         
         u8 pal_index = pal_index_a;
         if (y > switch_ab) {
            pal_index = pal_index_b;
            if (y > switch_bc) {
               pal_index = pal_index_c;
            }
         }
         
         if (!(x & 1)) { // LSBs = left; MSBs = right
            *dst_byte &= 0xF0;
            *dst_byte |= pal_index;
         } else {
            *dst_byte &= 0x0F;
            *dst_byte |= pal_index << 4;
         }
      }
   }
}

static void PlaceNthTile(struct LuScrollbar* widget, u8 n, u8 at, u8 count) {
   if (count <= 0) {
      return;
   }
   FillBgTilemapBufferRect(
      widget->graphics.bg_layer,
      widget->graphics.first_tile_id + n,
      widget->graphics.pos.x + at,
      widget->graphics.pos.y,
      1,
      count,
      widget->graphics.palette_id
   );
}

extern void RepaintScrollbarV(struct LuScrollbar* widget) {
   u8 track_height = widget->graphics.length * TILE_HEIGHT;
   u8 thumb_pos    = track_height * widget->scroll_pos / widget->item_count;
   u8 thumb_size   = track_height * widget->max_visible_items / widget->item_count;
   if (thumb_size == 0) {
      thumb_size = 1;
   }
   
   //
   // Each 8x8 tile uses four bits per pixel and so consumes 0x20 bytes. 
   // Pixels are arranged row-major.
   //
   if (widget->item_count <= widget->max_visible_items) {
      //
      // No scrolling is possible. Force all tiles to the background 
      // color.
      //
      FillTiles(widget->graphics.tile_buffer, widget->graphics.colors.background, SCROLLBAR_TILE_COUNT);
   } else {
      const u8 color_thumb = widget->graphics.colors.thumb;
      const u8 color_track = widget->graphics.colors.track;
      
      u8 tile_id = 0;
      
      bool8 thumb_at_start = (thumb_pos == 0);
      bool8 thumb_at_end   = (thumb_pos + thumb_size >= track_height);
      if (thumb_at_start || thumb_at_end) {
         //
         // The logic for both of these is essentially the same: the scrollbar 
         // is split into two colored regions ("prior" and "after"). Either of 
         // these regions MAY have at least one full tile; and the edge between 
         // the regions MAY be in the middle of a tile.
         //
         // This case ultimately requires at most 3 unique tile graphics.
         //
         u8 prior_color;
         u8 after_color;
         
         u8 prior_tile_count;
         u8 prior_tile_split;
         u8 after_tile_count;
         
         if (thumb_at_start) {
            prior_color = color_thumb;
            after_color = color_track;
            
            prior_tile_count = thumb_size / TILE_HEIGHT;
            prior_tile_split = thumb_size % TILE_HEIGHT;
            after_tile_count = widget->graphics.length - prior_tile_count;
         } else {
            prior_color = color_track;
            after_color = color_thumb;
            
            after_tile_count = thumb_size / TILE_HEIGHT;
            prior_tile_count = widget->graphics.length - after_tile_count;
            prior_tile_split = thumb_size % TILE_HEIGHT;
            if (prior_tile_split) {
               prior_tile_split = TILE_HEIGHT - prior_tile_split;
            }
         }
         if (prior_tile_split && after_tile_count > 0) {
            --after_tile_count;
         }
         
         u8 tile_y = 0;
         if (prior_tile_count) {
            PaintFullTile(GetNthTileData(widget, tile_id), prior_color);
            PlaceNthTile(widget, tile_id, tile_y, prior_tile_count);
            tile_y = prior_tile_count;
            ++tile_id;
         }
         if (prior_tile_split) {
            PaintEdgeTile(GetNthTileData(widget, tile_id), prior_color, after_color, prior_tile_split);
            PlaceNthTile(widget, tile_id, tile_y, 1);
            ++tile_id;
            ++tile_y;
         }
         if (after_tile_count) {
            PaintFullTile(GetNthTileData(widget, tile_id), after_color);
            PlaceNthTile(widget, tile_id, tile_y, after_tile_count);
            ++tile_id;
         }
      } else {
         //
         // The thumb is in the middle of the scrollbar.
         //
         // This case requires at most 4 unique tile graphics:
         //
         //    ─ Track middle
         //    ▐ Track-to-thumb
         //    █ Thumb middle
         //    ▌ Thumb-to-track
         //
         // Any of the following arrangements are possible:
         //
         //    ───▐█▌───
         //    ▐█▌──────
         //    ──────▐█▌
         //    ▐▌───────
         //    ───────▐▌
         //    ───▐█──── // thumb width > 8px; end of thumb tile-aligned
         //    ───▐▌────
         //    ───█▌──── // thumb width > 8px; start of thumb tile-aligned
         //    ───█───── // thumb width a multiple of 8; thumb tile-aligned
         //
         {
            //
            // Start by placing the all-track tiles.
            //
            const u8 track_before = thumb_pos;
            const u8 track_after  = (widget->graphics.length * TILE_HEIGHT) - (thumb_pos + thumb_size);
            
            const u8 tiles_before = track_before / TILE_HEIGHT;
            const u8 tiles_after  = track_after  / TILE_HEIGHT;
            //
            // Place the all-track tiles.
            //
            u8 track_tile_id = 0xFF;
            if (tiles_before) {
               u8 track_tile_count = thumb_pos / TILE_HEIGHT;
               
               PaintFullTile(GetNthTileData(widget, tile_id), color_track);
               PlaceNthTile(widget, tile_id, 1, track_tile_count);
               
               track_tile_id = tile_id;
               ++tile_id;
            }
            if (tiles_after) {
               if (track_tile_id == 0xFF) {
                  PaintFullTile(GetNthTileData(widget, tile_id), color_track);
                  track_tile_id = tile_id;
                  ++tile_id;
               }
               PlaceNthTile(widget, track_tile_id, widget->graphics.length - tiles_after, tiles_after);
            }
         }
         //
         // Next, place the thumb tile(s).
         //
         u8 thumb_tile_y1 = thumb_pos / TILE_HEIGHT;
         u8 thumb_tile_y2 = (thumb_pos + thumb_size) / TILE_HEIGHT;
         if (thumb_tile_y1 == thumb_tile_y2) {
            //
            // The scrollbar thumb fits entirely in the middle of a tile, i.e. 
            // we need a track/thumb/track double-edge tile.
            //
            u8 split_a = thumb_pos % TILE_HEIGHT;
            u8 split_b = (thumb_pos + thumb_size) % TILE_HEIGHT;
            
            PaintDoubleEdgeTile(GetNthTileData(widget, tile_id), color_track, color_thumb, color_track, split_a, split_b);
            PlaceNthTile(widget, tile_id, thumb_tile_y1, 1);
            ++tile_id;
         } else {
            //
            // The scrollbar thumb doesn't fit entirely within the middle of one 
            // tile. There may be all-thumb tiles, or the thumb may just occupy 
            // the bottom half of one tile and the top half of the next.
            //
            // The `thumb_tile_y1` variable tells us the first tile that should 
            // have any thumb pixels, and the `thumb_tile_y2` variable tells us 
            // the last tile that should have any thumb pixels.
            //
            if (thumb_tile_y2 > thumb_tile_y1) {
               //
               // The thumb covers two or more tiles, so there may be all-thumb 
               // tiles. For now, let's generate an all-thumb tile and then place 
               // it in all the spots where we know the thumb is present. If any 
               // of those spots should be edge tiles instead, we'll generate 
               // the edge tiles and replace tilemap entries as needed.
               //
               u8 thumb_tile_count = thumb_tile_y2 - thumb_tile_y1;
               PaintFullTile(GetNthTileData(widget, tile_id), color_thumb);
               PlaceNthTile(widget, tile_id, thumb_tile_y1, thumb_tile_count);
               ++tile_id;
            }
            {  // Track-to-thumb tile.
               u8 split = thumb_pos % TILE_HEIGHT;
               if (split) {
                  PaintEdgeTile(GetNthTileData(widget, tile_id), color_track, color_thumb, split);
                  PlaceNthTile(widget, tile_id, thumb_tile_y1, 1);
                  ++tile_id;
               }
            }
            {  // Thumb-to-track tile.
               u8 split = (thumb_pos + thumb_size) % TILE_HEIGHT;
               if (split) {
                  PaintEdgeTile(GetNthTileData(widget, tile_id), color_thumb, color_track, TILE_HEIGHT - split);
                  PlaceNthTile(widget, tile_id, thumb_tile_y2, 1);
                  ++tile_id;
               }
            }
         }
      }
   }
   
   
   // Copy updated tile graphics into VRAM.
   LoadBgTiles(
      widget->graphics.bg_layer,
      widget->graphics.tile_buffer,
      0x20 * 1 * SCROLLBAR_TILE_COUNT, // size of tile graphic * width * height
      widget->graphics.first_tile_id
   );
   
   // Update the VRAM-side tilemap. (Actually, nvm -- let the owning menu do this.)
   // CopyBgTilemapBufferToVram(widget->graphics.bg_layer);
}

extern void DestroyScrollbarV(struct LuScrollbar* widget) {
   if (!widget->graphics.tile_buffer) {
      Free(widget->graphics.tile_buffer);
      widget->graphics.tile_buffer = NULL;
   }
}