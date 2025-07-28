#include "lu/gfxutils.h"
#include "gba/gba.h"
#include <string.h> // compiler may need us to remove this if we ever #include "global.h"
#include "bg.h"
#include "decompress.h"
#include "window.h"

extern void FlipTile4bpp(u8* tile_data, bool8 flip_h, bool8 flip_v) {
   enum {
      BYTES_PER_ROW = 4,
   };
   #define XOR_SWAP(_a, _b)     \
      do {                      \
         __auto_type _0 = (_a); \
         __auto_type _1 = (_b); \
         _0 = _1 ^ _0;          \
         _1 = _0 ^ _1;          \
         _0 = _1 ^ _0;          \
         _a = _0;               \
         _b = _1;               \
      } while (0)
   
   if (flip_h) {
      //
      // Swap nybbles within each byte.
      //
      for(u8 i = 0; i < TILE_SIZE_4BPP; ++i) {
         u8 byte = tile_data[i];
         byte = (u8)(byte << 4) | (u8)(byte >> 4);
         tile_data[i] = byte;
      }
      //
      // Swap byte order for each row.
      //
      for(u8 y = 0; y < TILE_HEIGHT; ++y) {
         u8* row = &tile_data[y * BYTES_PER_ROW];
         XOR_SWAP(row[0], row[3]);
         XOR_SWAP(row[1], row[2]);
      }
   }
   if (flip_v) {
      //
      // Swap rows.
      //
      u32* rows = (u32*)tile_data;
      for(u8 y = 0, v = TILE_HEIGHT - 1; y < TILE_HEIGHT / 2; ++y, --v) {
         XOR_SWAP(rows[y], rows[v]);
      }
   }
   
   #undef XOR_SWAP
}

extern void FlipWindowTile(u8 window_id, u8 tile_x, u8 tile_y, bool8 flip_h, bool8 flip_v) {
   u8* tile_data = GetWindowTilePtr(window_id, tile_x, tile_y);
   if (!tile_data)
      return;
   FlipTile4bpp(tile_data, flip_h, flip_v);
}
extern u8* GetWindowTilePtr(u8 window_id, u8 tile_x, u8 tile_y) {
   u8* buffer = gWindows[window_id].tileData;
   if (!buffer)
      return NULL;
   u8 tile_index = tile_y * gWindows[window_id].window.width + tile_x;
   return &buffer[tile_index * TILE_SIZE_4BPP];
}

extern void DrawTiledBackground(
   const struct BGTilemapInfo* src,
   u8  dst_bg,
   u8  dst_first_palette_id,
   u16 dst_first_tile_id,
   s16 dst_tile_x,
   s16 dst_tile_y
) {
   enum {
      TILEMAP_ENTRY_SIZE = 2,
   };
   
   AGB_ASSERT(dst_bg < 4);
   AGB_ASSERT(dst_first_palette_id < 16);
   AGB_ASSERT(dst_first_tile_id < 1024);
   
   const u16 tile_count = src->size.w * src->size.h;
   AGB_ASSERT(tile_count * TILEMAP_ENTRY_SIZE <= sizeof(gDecompressionBuffer));
   
   u16* src_tilemap = (u16*) src->data.content;
   if (src->data.is_compressed) {
      src_tilemap = (u16*) gDecompressionBuffer;
      LZ77UnCompWram(src->data.content, gDecompressionBuffer);
   } else {
      memcpy(gDecompressionBuffer, src->data.content, tile_count * TILEMAP_ENTRY_SIZE);
   }
   //
   // Adjust palette ID and tile IDs.
   //
   {
      //                                 PPPPVHIIIIIIIIII
      const u16 MASK_STRIP_TILE_ID   = 0b1111110000000000;
      const u16 MASK_KEEP_ONLY_FLIPS = 0b0000110000000000;
      
      const u8  src_first_palette_id = src->data.first_palette_id;
      const u16 src_first_tile_id    = src->data.first_tile_id;
      
      const s16 tile_id_diff = dst_first_tile_id    - src_first_tile_id;
      const s8  pal_id_diff  = dst_first_palette_id - src_first_palette_id;
      
      if (dst_first_palette_id == src_first_palette_id) {
         for(u8 i = 0; i < tile_count; ++i) {
            u16* tile_ptr = &src_tilemap[i];
            u16  tile     = *tile_ptr;
            
            u16 tile_id = (tile & 1023) + tile_id_diff;
            
            tile &= MASK_STRIP_TILE_ID;
            tile |= (tile_id & 1023);
            
            *tile_ptr = tile;
         }
      } else {
         for(u8 i = 0; i < tile_count; ++i) {
            u16* tile_ptr = &src_tilemap[i];
            u16  tile     = *tile_ptr;
            
            u16 tile_id  = (tile & 1023)       + tile_id_diff;
            u8  tile_pal = ((tile >> 12) & 15) + pal_id_diff;
            
            //        PPPPVHIIIIIIIIII
            tile &= MASK_KEEP_ONLY_FLIPS;
            tile |= (u16)(tile_pal & 15) << 12;
            tile |= (tile_id & 1023);
            
            *tile_ptr = tile;
         }
      }
   }
   //
   // Write to BG layer.
   //
   u16* dst_tilemap = (u16*) GetBgTilemapBuffer(dst_bg);
   if (!dst_tilemap) {
      AGB_WARNING(dst_tilemap != NULL && "The BG tilemap must already be prepped!");
      return;
   }
   
   u16 src_map_x;
   u16 src_map_y;
   //
   if (dst_tile_x < 0) {
      src_map_x = -dst_tile_x;
   } else {
      src_map_x = dst_tile_x;
   }
   src_map_x %= src->size.w;
   //
   if (dst_tile_y < 0) {
      src_map_y = -dst_tile_y;
   } else {
      src_map_y = dst_tile_y;
   }
   src_map_y %= src->size.h;
   
   u16 dst_map_w = GetBgMetricTextMode(dst_bg, 0x1) * (256 / TILE_WIDTH);
   u16 dst_map_h = GetBgMetricTextMode(dst_bg, 0x2) * (256 / TILE_HEIGHT);
   {
      const u16 src_tile_w = src->size.w;
      const u16 src_tile_h = src->size.h;
      for(u16 y = 0; y < dst_map_h; ++y) {
         const u16* src_row = (const u16*) &src_tilemap[src_map_y * src_tile_w];
         u16 sx = 0;
         for(u16 x = 0; x < dst_map_w; ++x) {
            dst_tilemap[y * dst_map_w + x] = src_row[sx];
            
            ++sx;
            sx %= src_tile_w;
         }
         ++src_map_y;
         src_map_y %= src_tile_h;
      }
   }
   CopyBgTilemapBufferToVram(dst_bg);
}