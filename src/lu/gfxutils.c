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

extern void BlitBitmapRect4BitRemapped(
   const struct Bitmap* src,
   struct Bitmap*       dst,
   u16 src_x,
   u16 src_y,
   u16 dst_x,
   u16 dst_y,
   u16 width,
   u16 height,
   const u8* color_mapping
) {
   s32 src_x_end;
   s32 src_y_end;
   if (dst->width - dst_x < width)
      src_x_end = (dst->width - dst_x) + src_x;
   else
      src_x_end = src_x + width;
   
   if (dst->height - dst_y < height)
      src_y_end = (dst->height - dst_y) + src_y;
   else
      src_y_end = height + src_y;
   
   //
   // A four-bits-per-pixel image stores two pixels per byte. The low nybble 
   // is the left-half palette color, and the high nybble is the right-half 
   // palette color.
   //
   // The naive approach would be to just loop over the columns and rows, 
   // split source bytes in half, remap each color, and merge them back to 
   // a single byte to write into the destination.
   //
   // Just one problem: What if the source-X coordinate is an odd number? 
   // What if the destination-X coordinate is an odd number?
   //
   // The simplest approach, then, is to go one pixel at a time and thus 
   // one nybble at a time. This is simple, but it's not *efficient*. It 
   // may be better to just handle each possible alignment separately... 
   // except that there's one more alignment to consider: tile alignment. 
   // Graphics are broken into 8x8px tiles. So given a graphic like this:
   //
   //     ABCDEFGHIJKLMNOP
   //     QRSTUVWXYZ123456
   //     ...
   //
   // The actual arrangement of pixel data is as follows:
   //
   //    |BA|DC|FE|HG|
   //    |RQ|TS|VU|XW|
   //    |..|..|..|..|
   //    |..|..|..|..|
   //    |..|..|..|..|
   //    |..|..|..|..|
   //    |..|..|..|..|
   //    |..|..|..|..|
   //    |JI|LK|NM|PO|
   //    |ZY|21|43|65|
   //    |..|..|..|..|
   //    |..|..|..|..|
   //    |..|..|..|..|
   //    |..|..|..|..|
   //    |..|..|..|..|
   //    |..|..|..|..|
   //
   // So given pixel coordinates X and Y, in a graphic whose size in pixels 
   // is W by H, the location of the pixel's containing byte is:
   //
   //    pixel_data_start + (
   //       (Y / TILE_HEIGHT) * (W / TILE_WIDTH) * TILE_SIZE_4BPP
   //     + (Y % TILE_HEIGHT) * (TILE_WIDTH / 2)
   //     + (X / TILE_WIDTH)  * TILE_SIZE_4BPP
   //     + (X % TILE_WIDTH)  / 2
   //    )
   //
   
   #define PIXEL_L(duo)      ((duo) & 15)
   #define PIXEL_R(duo)      ((duo) >> 4)
   #define MERGE_PIXEL(l, r) (((l) & 15) | ((r) << 4))
   #define each_row          (; loop_src_y < src_y_end; ++loop_src_y, ++loop_dst_y)
   #define each_pair         (; dx + 1 < width; dx += 2)
   
   s32 loop_src_y = src_y;
   s32 loop_dst_y = dst_y;
   u16 dx;
   const u8* src_px;
   u8*       dst_px;
   
   const bool8 dst_misaligned = (dst_x & 1);
   const bool8 src_misaligned = (src_x & 1);
   
   const u8  PIXELS_PER_BYTE         = 2;
   const u8  BYTES_PER_TILE_ROW      = TILE_WIDTH / PIXELS_PER_BYTE;
   
   const u8* _find_tile_byte(const u8* buffer, u16 x_px, u16 y_px, u16 w_px) {
      return &buffer[
         (y_px / TILE_HEIGHT) * (w_px / TILE_WIDTH) * TILE_SIZE_4BPP
       + (y_px % TILE_HEIGHT) * BYTES_PER_TILE_ROW
       + (x_px / TILE_WIDTH) * TILE_SIZE_4BPP
       + (x_px % TILE_WIDTH) / PIXELS_PER_BYTE
      ];
   }
   inline void _update_pixel_pair_pointers(void) {
      src_px = _find_tile_byte(
         src->pixels,
         src_x + dx,
         loop_src_y,
         src->width
      );
      dst_px = (u8*) _find_tile_byte(
         dst->pixels,
         dst_x + dx,
         loop_dst_y,
         dst->width
      );
   }
   
   if (dst_misaligned == src_misaligned) {
      //
      // The source and destination X-coordinates both have the same align-
      // ment with respect to two-pixel (one-byte) boundaries.
      //
      inline void _blit_src_l_to_dst_l(void) {
         _update_pixel_pair_pointers();
         const u8 src_duo = *src_px;
         const u8 dst_duo = *dst_px;
         u8 color_l = color_mapping[PIXEL_L(src_duo)];
         u8 color_r = PIXEL_R(dst_duo);
         *dst_px = MERGE_PIXEL(color_l, color_r);
         ++dx;
      }
      inline void _blit_src_r_to_dst_r(void) {
         _update_pixel_pair_pointers();
         const u8 src_duo = *src_px;
         const u8 dst_duo = *dst_px;
         u8 color_l = PIXEL_L(dst_duo);
         u8 color_r = color_mapping[PIXEL_R(src_duo)];
         *dst_px = MERGE_PIXEL(color_l, color_r);
         ++dx;
      }
      inline void _looped_blit_matched_pair(void) {
         _update_pixel_pair_pointers();
         const u8 src_duo = *src_px;
         u8 color_l = color_mapping[PIXEL_L(src_duo)];
         u8 color_r = color_mapping[PIXEL_R(src_duo)];
         *dst_px = MERGE_PIXEL(color_l, color_r);
      }
      
      if (dst_misaligned) {
         //
         // The source and destination X-corodinates are both misaligned 
         // from a two-pixel boundary. For each row, we can blit the first 
         // pixel on its own; then blit two pixels at a time; and then if 
         // the width is an even number, we can blit the last pixel of the 
         // row on its own too.
         //
         for each_row {
            dx = 0;
            _blit_src_r_to_dst_r();
            for each_pair {
               _looped_blit_matched_pair();
            }
            if (dx < width)
               _blit_src_l_to_dst_l();
         }
      } else {
         //
         // The source and destination X-coordinates are both aligned to 
         // a two-pixel boundary. For each row, we can blit two pixels 
         // at a time; and if the width is an odd number, then we can 
         // blit the last pixel of the row on its own.
         //
         for each_row {
            dx = 0;
            for each_pair {
               _looped_blit_matched_pair();
            }
            if (dx < width)
               _blit_src_l_to_dst_l();
         }
      }
   } else {
      //
      // The source and destination X-coordinates are aligned differently 
      // from one another, with respect to two-pixel boundaries. Broadly 
      // speaking, the blitting will fall into this pattern:
      //
      //    Dst Target:       |       |
      //    Src Pixels: |L|R|L|R|L|R|L|R|L|R|L|R|
      //    Dst Pixels: |L|R|L|R|L|R|L|R|L|R|L|R|
      //    Src Buffer: |R L|R L|R L|R L|R L|R L|
      //    Dst Buffer: |. .|. R|L R|L .|. .|. .| (blitted src pixels only)
      //
      inline void _blit_src_l_to_dst_r(void) {
         _update_pixel_pair_pointers();
         const u8 src_duo = *src_px;
         const u8 dst_duo = *dst_px;
         u8 color_l = PIXEL_L(dst_duo);
         u8 color_r = color_mapping[PIXEL_L(src_duo)];
         *dst_px = MERGE_PIXEL(color_l, color_r);
         ++dx;
      }
      inline void _blit_src_r_to_dst_l(void) {
         _update_pixel_pair_pointers();
         const u8 src_duo = *src_px;
         const u8 dst_duo = *dst_px;
         u8 color_l = color_mapping[PIXEL_R(src_duo)];
         u8 color_r = PIXEL_R(dst_duo);
         *dst_px = MERGE_PIXEL(color_l, color_r);
         ++dx;
      }
      inline void _looped_blit_mismatched_pair(void) {
         _update_pixel_pair_pointers();
         const u8 src_duo = *src_px;
         u8 color_l = color_mapping[PIXEL_L(src_duo)];
         u8 color_r = color_mapping[PIXEL_R(src_duo)];
         *dst_px = MERGE_PIXEL(color_r, color_l);
      }
      
      if (dst_misaligned) {
         //
         // The source rect is aligned to a two-pixel boundary, but the 
         // destination rect is not. We bit mismatched pairs, and then 
         // potentially blit src-L to dst-R.
         //
         for each_row {
            dx = 0;
            for each_pair {
               _looped_blit_mismatched_pair();
            }
            if (dx < width)
               _blit_src_l_to_dst_r();
         }
      } else {
         //
         // The destination rect is aligned to a two-pixel boundary, but 
         // the source rect is not. We begin by blitting src-R to dst-L, 
         // and then blit mismatched pairs. If there's a trailing pixel 
         // left after the pairs, it will be src-L, to be blit to dst-R.
         //
         for each_row {
            dx = 0;
            _blit_src_r_to_dst_l();
            for each_pair {
               _looped_blit_mismatched_pair();
            }
            if (dx < width)
               _blit_src_l_to_dst_r();
         }
      }
   }
   
   #undef PIXEL_L
   #undef PIXEL_R
   #undef MERGE_PIXEL
   #undef each_row
   #undef each_pair
}

extern void BlitTile4BitRemapped(
   const u8* src,
   u8*       dst,
   const u8* color_mapping
) {
   for(u8 i = 0; i < TILE_SIZE_4BPP; ++i) {
      u8 duo     = src[i];
      u8 color_l = color_mapping[duo & 15];
      u8 color_r = color_mapping[duo >> 4];
      dst[i] = (color_r << 4) | (color_l & 15);
   }
}
