#ifndef GUARD_LU_VUI_FRAME_H
#define GUARD_LU_VUI_FRAME_H

#include "gba/types.h"
#include "lu/vui/vui-types.h"
#include "lu/c-attr.define.h"

enum {
   VUIFRAME_TILEID_SENTINEL  = 0xF000,
   VUIFRAME_TILEID_UNCHANGED = VUIFRAME_TILEID_SENTINEL | 0, // "don't draw a tile here"
};
enum {
   //
   // Any corner can be set to take on the appearance of any other corner, 
   // with a horizontal/vertical flip as appropriate. If your frame has 
   // multiple corners that are visually identical save for that flip, 
   // then use these constants instead of defining the tile ID array for 
   // each of them.
   //
   VUIFRAME_CORNER_MIRROR_X    = 0xFF000001,
   VUIFRAME_CORNER_MIRROR_Y    = 0xFF000002,
   VUIFRAME_CORNER_MIRROR_BOTH = 0xFF000003,
};

typedef union VUIFrameCornerTileInfo {
   const u16* ids;    // row-major order
   u32        mirror; // VUIFRAME_CORNER_MIRROR_...
} VUIFrameCornerTileInfo;

typedef struct VUIFrameEdge {
   u16   tile_id : 10;
   bool8 flip_h  :  1;
   bool8 flip_v  :  1;
   bool8 mirror  :  1; // ignore above params and use the tile of the opposite edge, flipped
} VUIFrameEdge;

typedef struct VUIFrame {
   u8 bg_layer : 2;
   u8 palette  : 4;
   struct {
      union {
         VUISize list[4];
         struct {
            VUISize tl;
            VUISize tr;
            VUISize bl;
            VUISize br;
         };
      } sizes;
      union {
         VUIFrameCornerTileInfo list[4];
         struct {
            VUIFrameCornerTileInfo tl;
            VUIFrameCornerTileInfo tr;
            VUIFrameCornerTileInfo bl;
            VUIFrameCornerTileInfo br;
         };
      } tiles;
   } corners;
   union {
      VUIFrameEdge list[4];
      struct {
         VUIFrameEdge left;
         VUIFrameEdge right;
         VUIFrameEdge top;
         VUIFrameEdge bottom;
      };
   } edges;
} VUIFrame;

// Args measured in tiles.
NON_NULL_PARAMS(1)
extern void VUIFrame_Draw(const VUIFrame*, const u8 x, const u8 y, const u8 w, const u8 h);

#include "lu/c-attr.undef.h"
#endif