#include "lu/gfxutils.h"
#include "window.h"
#include "gba/defines.h"

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