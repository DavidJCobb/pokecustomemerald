#ifndef GUARD_LU_GFXUTILS_H
#define GUARD_LU_GFXUTILS_H

#include "gba/types.h"
#include "blit.h"

extern void FlipTile4bpp(u8* tile_data, bool8 flip_h, bool8 flip_v);

extern void FlipWindowTile(u8 window_id, u8 tile_x, u8 tile_y, bool8 flip_h, bool8 flip_v);
extern u8* GetWindowTilePtr(u8 window_id, u8 tile_x, u8 tile_y);

struct BGTilemapInfo {
   struct {
      const u32* content; // must be aligned if compressed
      bool8      is_compressed    :  1;
      u8         first_palette_id :  4;
      u16        first_tile_id    : 10;
   } data;
   struct {
      u16 w;
      u16 h;
   } size; // measured in tiles
};

// Tile a single tilemap across an entire BG layer.
extern void DrawTiledBackground(
   const struct BGTilemapInfo*,
   u8  dst_bg,
   u8  dst_first_palette_id,
   u16 dst_first_tile_id,
   s16 dst_tile_x, // Location of the first Tile (0, 0) in the source tilemap.
   s16 dst_tile_y
);

// Blit a 4bpp bitmap onto another 4bpp bitmap. The `color_mapping` parameter 
// should be a pointer to an array of 16 palette color indices, such that the 
// color N in the source image is blitted into the destination as the color 
// `color_mapping[N]`.
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
);

extern void BlitTile4BitRemapped(
   const u8* src,
   u8*       dst,
   const u8* color_mapping
);

#endif