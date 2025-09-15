#include "lu/gfxutils.h"
#include "gba/gba.h"
#include <string.h> // compiler may need us to remove this if we ever #include "global.h"
#include "lu/c.h"
#include "bg.h"
#include "decompress.h"
#include "window.h"

extern void FlipTile4bpp(u8* tile_data, bool8 flip_h, bool8 flip_v) {
   enum {
      BYTES_PER_ROW = 4,
   };
   #define SWAP(_a, _b)         \
      do {                      \
         __auto_type _0 = (_a); \
         __auto_type _1 = (_b); \
         _a = _1;               \
         _b = _0;               \
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
         SWAP(row[0], row[3]);
         SWAP(row[1], row[2]);
      }
   }
   if (flip_v) {
      //
      // Swap rows.
      //
      u32* rows = (u32*)tile_data;
      for(u8 y = 0, v = TILE_HEIGHT - 1; y < TILE_HEIGHT / 2; ++y, --v) {
         SWAP(rows[y], rows[v]);
      }
   }
   
   #undef SWAP
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
   const u8 color_mapping[]
) {
   if (src_x >= src->width)
      return;
   if (src_x + width > src->width)
      width = src->width - src_x;
   if (dst_x >= dst->width)
      return;
   if (dst_x + width > dst->width)
      width = dst->width - dst_x;
   
   if (src_y > src->height)
      return;
   if (src_y + height > src->height)
      height = src->height - src_y;
   if (dst_y >= dst->height)
      return;
   if (dst_y + height > dst->height)
      height = dst->height - dst_y;
   
   s32 src_x_end;
   s32 src_y_end;
   src_x_end = src_x + width;
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
   // So if we use a nested loop -- Y outer, X inner -- then we can find 
   // the start of each row in the outer loop, and then find the target 
   // pixels' containing bytes in the inner loop.
   //
   
   #define PIXEL_L(duo)      ((duo) & 15)
   #define PIXEL_R(duo)      ((duo) >> 4)
   #define MERGE_PIXEL(l, r) (((l) & 15) | ((r) << 4))
   #define each_row          (; loop_src_y < src_y_end; ++loop_src_y, ++loop_dst_y)
   #define each_pair         (; dx + 1 < width; dx += 2)
   
   enum {
      PIXELS_PER_BYTE    = 2,
      BYTES_PER_TILE_ROW = TILE_WIDTH / PIXELS_PER_BYTE,
   };
   #define PIXEL_ROW_BYTE(buffer, y_px, w_px) \
      (&(buffer)[ \
         ((y_px) / TILE_HEIGHT) * ((w_px) / TILE_WIDTH) * TILE_SIZE_4BPP \
       + ((y_px) % TILE_HEIGHT) * BYTES_PER_TILE_ROW \
      ])
   #define PIXEL_BYTE(pixel_row_buffer, x_px)    \
      (&(pixel_row_buffer)[                      \
         ((x_px) / TILE_WIDTH) * TILE_SIZE_4BPP  \
       + ((x_px) % TILE_WIDTH) / PIXELS_PER_BYTE \
      ])
   
   s32 loop_src_y = src_y;
   s32 loop_dst_y = dst_y;
   u16 dx;
   
   const u8* src_px_row;
   u8*       dst_px_row;
   
   inline void _find_pixel_row_pointers(void) {
      src_px_row = PIXEL_ROW_BYTE(src->pixels, loop_src_y, src->width);
      dst_px_row = PIXEL_ROW_BYTE(dst->pixels, loop_dst_y, dst->width);
   }
   
   #define SRC_PIXEL     *(PIXEL_BYTE(src_px_row, src_x + dx))
   #define DST_PIXEL_PTR PIXEL_BYTE(dst_px_row, dst_x + dx)
   
   //
   // So. The blitting macros.
   //
   // First, we have "TRANSFORM" macros. These are just used to handle the 
   // color remapping when reading the source pixel.
   //
   // Next, the "BLIT" macros themselves.
   //
   // When the source and destination rects have the same two-pixel alignment, 
   // we can, in general, blit pixels two at a time (i.e. blit one byte at a 
   // time). If both rects are misaligned, then we'll have to blit the leading 
   // pixel on its own (i.e. half a byte); and no matter how they're aligned, 
   // we may have to blit a trailing pixel (e.g. if both rects are aligned but 
   // the blit width is odd; or if both rects are misaligned and the blit width 
   // is even).
   //
   // The "aligned" case looks like this:
   //
   //    Dst Target:         |       |
   //    Src Pixels: |L|R|L|R|L|R|L|R|L|R|L|R|
   //    Dst Pixels: |L|R|L|R|L|R|L|R|L|R|L|R|
   //    Src Buffer: |R L|R L|R L|R L|R L|R L|
   //    Dst buffer: |. .|. .|R L|R L|. .|. .|
   //
   // The "misaligned" case looks like this:
   //
   //    Dst Target:       |       |  
   //    Src Pixels: |L|R|L|R|L|R|L|R|L|R|L|R|
   //    Dst Pixels: |L|R|L|R|L|R|L|R|L|R|L|R|
   //    Src Buffer: |R L|R L|R L|R L|R L|R L|
   //    Dst buffer: |. .|R .|R L|. L|. .|. .|
   //
   // When the source and destination rects don't have the same two-pixel 
   // alignment -- when one is aligned and the other is not -- things get a 
   // bit trickier. We can still blit most of each pixel row in pairs, but 
   // the order of each pair's nybbles must be swapped. This situation will 
   // generally look like this:
   //
   //    Dst Target:       |       |  
   //    Src Pixels: |L|R|L|R|L|R|L|R|L|R|L|R|
   //    Dst Pixels: |L|R|L|R|L|R|L|R|L|R|L|R|
   //    Src Buffer: |R L|R L|R L|R L|R L|R L|
   //    Dst buffer: |. .|. R|L R|L .|. .|. .|
   //
   // We call these "mismatched pairs," as opposed to the "matched pairs" 
   // (that don't need nybble swapping) that we blit when the source and 
   // destination rects have the same (mis)alignment.
   //
   // If the source is misaligned, then we blit the source's rightmost pixel 
   // in isolation, overwriting the lefthand pixel in a destination pair. 
   // Either way, we then blit mismatched pairs from the source to the 
   // destination; and then if there remains a trailing pixel, we blit that 
   // with a similar swap (i.e. source-pair-left to destination-pair-right).
   //
   
   #define TRANSFORM_src(v) color_mapping[(v)]
   #define TRANSFORM_dst(v) (v)
   #define SINGLE_PIXEL_BLIT(left_subject, left_side, right_subject, right_side) \
      do { \
         auto dst_ptr = DST_PIXEL_PTR; \
         auto src_duo = SRC_PIXEL; \
         auto dst_duo = *dst_ptr; \
         u8 color_l = TRANSFORM_##left_subject(PIXEL_##left_side(left_subject##_duo)); \
         u8 color_r = TRANSFORM_##right_subject(PIXEL_##right_side(right_subject##_duo)); \
         *dst_ptr = MERGE_PIXEL(color_l, color_r); \
         ++dx; \
      } while (0)
   
   #define BLIT_SRC_L_TO_DST_L SINGLE_PIXEL_BLIT(src, L, dst, R)
   #define BLIT_SRC_L_TO_DST_R SINGLE_PIXEL_BLIT(dst, L, src, L)
   #define BLIT_SRC_R_TO_DST_R SINGLE_PIXEL_BLIT(dst, L, src, R)
   #define BLIT_SRC_R_TO_DST_L SINGLE_PIXEL_BLIT(src, R, dst, R)
   
   #define PIXEL_PAIR_BLIT(left_side, right_side) \
      do { \
         auto src_duo = SRC_PIXEL; \
         u8 color_l = TRANSFORM_src(PIXEL_##left_side(src_duo)); \
         u8 color_r = TRANSFORM_src(PIXEL_##right_side(src_duo)); \
         *DST_PIXEL_PTR = MERGE_PIXEL(color_l, color_r); \
      } while (0)
   
   #define BLIT_MATCHED_PAIR    PIXEL_PAIR_BLIT(L, R)
   #define BLIT_MISMATCHED_PAIR PIXEL_PAIR_BLIT(R, L)
   
   const bool8 dst_misaligned = (dst_x & 1);
   const bool8 src_misaligned = (src_x & 1);
   if (dst_misaligned == src_misaligned) {
      //
      // Source and destination have the same (mis)alignment. BLitting can 
      // preserve nybble order from source to destination.
      //
      if (dst_misaligned) {
         for each_row {
            dx = 0;
            _find_pixel_row_pointers();
            BLIT_SRC_R_TO_DST_R;
            for each_pair {
               BLIT_MATCHED_PAIR;
            }
            if (dx < width)
               BLIT_SRC_L_TO_DST_L;
         }
      } else {
         for each_row {
            dx = 0;
            _find_pixel_row_pointers();
            for each_pair {
               BLIT_MATCHED_PAIR;
            }
            if (dx < width)
               BLIT_SRC_L_TO_DST_L;
         }
      }
   } else {
      //
      // Source and destination are aligned differently. Blitting will 
      // need to swap the source nybble order en route to the destination.
      //
      if (dst_misaligned) {
         for each_row {
            dx = 0;
            _find_pixel_row_pointers();
            for each_pair {
               BLIT_MISMATCHED_PAIR;
            }
            if (dx < width)
               BLIT_SRC_L_TO_DST_R;
         }
      } else {
         for each_row {
            dx = 0;
            _find_pixel_row_pointers();
            BLIT_SRC_R_TO_DST_L;
            for each_pair {
               BLIT_MISMATCHED_PAIR;
            }
            if (dx < width)
               BLIT_SRC_L_TO_DST_R;
         }
      }
   }
   
   #undef PIXEL_L
   #undef PIXEL_R
   #undef MERGE_PIXEL
   #undef each_row
   #undef each_pair
   #undef PIXEL_ROW_BYTE
   #undef PIXEL_BYTE
   
   #undef TRANSFORM_src
   #undef TRANSFORM_dst
   #undef SINGLE_PIXEL_BLIT
   #undef BLIT_SRC_L_TO_DST_L
   #undef BLIT_SRC_L_TO_DST_R
   #undef BLIT_SRC_R_TO_DST_R
   #undef BLIT_SRC_R_TO_DST_L
   #undef PIXEL_PAIR_BLIT
   #undef BLIT_MATCHED_PAIR
   #undef BLIT_MISMATCHED_PAIR
}

extern void BlitTile4BitRemapped(
   const u8* src,
   u8*       dst,
   const u8  color_mapping[]
) {
   for(u8 i = 0; i < TILE_SIZE_4BPP; ++i) {
      u8 duo     = src[i];
      u8 color_l = color_mapping[duo & 15];
      u8 color_r = color_mapping[duo >> 4];
      dst[i] = (color_r << 4) | (color_l & 15);
   }
}

extern void PrepBgTilemap(
   u8 bg,
   const u16* tilemap_src,
   u16 tilemap_size,
   u16 shift_tile_ids_by
) {
   auto tilemap_dst = (u16*)GetBgTilemapBuffer(bg);
   if (tilemap_dst == NULL)
      return;
   
   //                                 PPPPVHIIIIIIIIII
   const u16 MASK_STRIP_TILE_ID   = 0b1111110000000000;
   const u16 MASK_KEEP_ONLY_FLIPS = 0b0000110000000000;
   
   #define STRICT 0
   
   tilemap_size /= sizeof(u16);
   for(u16 i = 0; i < tilemap_size; ++i, ++tilemap_src, ++tilemap_dst) {
      u16 tile_item = *tilemap_src;
      #if STRICT
         u16 tile_id = (tile_item + shift_tile_ids_by) & ~MASK_STRIP_TILE_ID;
         tile_item &= MASK_STRIP_TILE_ID;
         tile_item |= tile_id;
      #else
         tile_item += shift_tile_ids_by;
      #endif
      *tilemap_dst = tile_item;
   }
   
   #undef STRICT
}
extern void PrepBgTilemapWithPalettes(
   u8 bg,
   const u16* tilemap_src,
   u16 tilemap_size,
   u16 shift_tile_ids_by,
   u8  shift_palette_ids_by
) {
   auto tilemap_dst = (u16*)GetBgTilemapBuffer(bg);
   if (tilemap_dst == NULL)
      return;
   
   tilemap_size /= sizeof(u16);
   for(u16 i = 0; i < tilemap_size; ++i, ++tilemap_src, ++tilemap_dst) {
      u16 tile_item  = *tilemap_src;
      u8  palette_id = tile_item >> 12;
      palette_id += shift_palette_ids_by;
      tile_item += shift_tile_ids_by;
      tile_item &= ~(15 << 12);
      tile_item |= (u16)palette_id << 12;
      *tilemap_dst = tile_item;
   }
}
