#ifndef GUARD_LU_GFXUTILS_H
#define GUARD_LU_GFXUTILS_H

#include "gba/types.h"
#include "blit.h"

extern void FlipTile4bpp(u8* tile_data, bool8 flip_h, bool8 flip_v);

extern void FlipWindowTile(u8 window_id, u8 tile_x, u8 tile_y, bool8 flip_h, bool8 flip_v);
extern u8* GetWindowTilePtr(u8 window_id, u8 tile_x, u8 tile_y);

#endif