#ifndef GUARD_LU_VRAM_HELPERS_NEW
#define GUARD_LU_VRAM_HELPERS_NEW

#include "gba/types.h"
#include "gba/defines.h"

typedef struct {
   u8 bytes[TILE_SIZE_4BPP];
} ALIGNED(TILE_SIZE_4BPP) vram_bg_tile;

typedef struct {
   u8 bytes[TILE_SIZE_8BPP];
} ALIGNED(TILE_SIZE_8BPP) vram_bg_tile_256; // 256-color

typedef struct {
   u8 bytes[BG_SCREEN_SIZE];
} ALIGNED(BG_SCREEN_SIZE) vram_bg_tilemap;

#define vram_bg_layout struct __vram_bg_layout
#define __verify_vram_bg_layout \
   _Static_assert(sizeof(vram_bg_layout) <= BG_VRAM_SIZE, "VRAM layout is too large!")

/*

   Usage:
   
      vram_bg_layout {
         __vram_bg_tile    my_tile;
         __vram_bg_tilemap my_tilemap;
      };
      __verify_vram_bg_layout;

*/

#define V_TILE_ID(member) (offsetof(vram_bg_layout, member) / TILE_SIZE_4BPP)
#define V_TILE_COUNT(member) (sizeof( ((vram_bg_layout*)NULL)-> member ) / TILE_SIZE_4BPP)

#define V_CHAR_BASE(member) (offsetof(vram_bg_layout, member) / BG_CHAR_SIZE)
#define V_MAP_BASE(member) (offsetof(vram_bg_layout, member) / BG_SCREEN_SIZE)

#define V_CHAR_BASED_TILE_ID(char_base_index, member) (V_TILE_ID(member) - (char_base_index * (BG_CHAR_SIZE / TILE_SIZE_4BPP)))

// Boolean. Checks whether the tileset beginning at a given char-base can access the 
// given tile(s) member.
#define V_TILESET_CAN_ACCESS(char_base_index, tile) (V_TILE_ID(tile) >= (char_base_index * 512) && V_TILE_ID(tile) + V_TILE_COUNT(tile) < 1024)

#define V_LOAD_TILES(bg_layer, tiles_member, src) \
   do { \
      _Static_assert(sizeof(src) == sizeof((( vram_bg_layout *)NULL)-> tiles_member ), "size of tile source data doesn't match size allotted in your VRAM layout"); \
      LoadBgTiles((bg_layer), (src), sizeof(src), V_TILE_ID(tiles_member)); \
   } while (0)

#define V_LOAD_COMPRESSED(tiles_member, src) \
   do { LZ77UnCompVram(src, (void*) (BG_VRAM + offsetof(vram_bg_layout, member)) ); } while (0)

#define V_SET_TILE(bg, tile_id, x, y, palette) \
   do { \
      FillBgTilemapBufferRect(bg, tile_id, x, y, 1, 1, palette); \
   } while (0)

#endif