#include "lu/vui/vui-frame.h"
#include "lu/c.h"
#include "gba/gba.h"
#include "bg.h"

#define max(_a, _b) ({ auto _u = (_a); auto _v = (_b); (_u < _v) ? _v : _u; })

inline static bool8 CornerIsMirrored(const VUIFrameCornerTileInfo* corner) {
   return (corner->mirror >> 24) == 0xFF;
}

extern void VUIFrame_Draw(const VUIFrame* this, const u8 x, const u8 y, const u8 w, const u8 h) {
   u8 bg      = this->bg_layer;
   u8 palette = this->palette;
   #ifndef NDEBUG
      //
      // Verify that the VUIFrame is well-formed.
      //
      {  // Corners.
         for(u8 i = 0; i < 4; ++i) {
            auto corner = &this->corners.tiles.list[i];
            auto mirror = corner->mirror;
            if (CornerIsMirrored(corner)) {
               u8 seen = (1 << i);
               do {
                  u8 j = mirror & 3;
                  switch (mirror) {
                     case VUIFRAME_CORNER_MIRROR_X:
                     case VUIFRAME_CORNER_MIRROR_Y:
                     case VUIFRAME_CORNER_MIRROR_BOTH:
                        break;
                     default:
                        AGB_WARNING(FALSE && "VUIFrame: Unrecognized corner-mirror mode! Expect undefined mirroring behavior.");
                        break;
                  }
                  j ^= i;
                  if (seen & (1 << j)) {
                     AGB_ASSERT(FALSE && "VUIFrame: Cyclical references between corners are not allowed. The draw code code will loop infinitely!");
                     break;
                  }
                  seen |= (1 << j);
                  corner = &this->corners.tiles.list[j];
               } while (CornerIsMirrored(corner));
               continue;
            }
            AGB_ASSERT(this->corners.tiles.list[i].ids != NULL);
         }
      }
      for(u8 i = 0; i < 2; ++i) {
         bool8 mirror_a = this->edges.list[i * 2 + 0].mirror;
         bool8 mirror_b = this->edges.list[i * 2 + 1].mirror;
         AGB_WARNING(!(mirror_a && mirror_b) && "Two opposing edges can't both mirror each other!");
      }
   #endif
   
   #define PAINT(_tile, _x, _y, _w, _h) \
      do { \
         auto t = (_tile); \
         if (t == VUIFRAME_TILEID_UNCHANGED) \
            break; \
         FillBgTilemapBufferRect(bg, t, (_x), (_y), (_w), (_h), palette); \
      } while (0)

   // Edge extents.
   u8 inner_x[2] = { x, x }; // top and bottom
   u8 inner_w[2] = { w, w };
   u8 inner_y[2] = { y, y }; // left and right
   u8 inner_h[2] = { h, h };

   // Corners:
   for(u8 i = 0; i < 4; ++i) {
      u8 c_lr = i % 2;
      u8 c_tb = i / 2;
      
      VUISize    size;
      const u16* tile_ids  = NULL;
      u16        tile_flip = 0;
      {
         u8   j;
         auto corner = &this->corners.tiles.list[i];
         while (CornerIsMirrored(corner)) {
            j          = corner->mirror & 3;
            tile_flip ^= ((u16)j << 10);
            j         ^= i;
            corner     = &this->corners.tiles.list[j];
         }
         tile_ids = corner->ids;
         size = this->corners.sizes.list[j];
      }
      if (!size.w || !size.h)
         continue;
      AGB_ASSERT(tile_ids != NULL);
      
      if (!c_lr)
         inner_x[c_tb] += size.w;
      inner_w[c_tb] -= size.w;
      if (!c_tb)
         inner_y[c_lr] += size.h;
      inner_h[c_lr] -= size.h;
      
      u8 cx = c_lr ? x + w - size.w : x;
      u8 cy = c_tb ? y + h - size.h : y;
      for(u8 tx = 0; tx < size.w; ++tx) {
         for(u8 ty = 0; ty < size.h; ++ty) {
            u16 tile_id = tile_ids[ty*size.w + tx] ^ tile_flip;
            u8  dx      = cx + tx;
            u8  dy      = cy + ty;
            if (tile_flip & (1 << 10)) {
               dx = cx + (size.w - tx - 1);
            }
            if (tile_flip & (1 << 11)) {
               dy = cy + (size.h - ty - 1);
            }
            
            PAINT(tile_id, dx, dy, 1, 1);
         }
      }
   }
   
   // Edges:
   for(u8 i = 0; i < 4; ++i) {
      u16  tile_id = 0;
      auto edge    = &this->edges.list[i];
      if (edge->mirror) {
         {
            u8 j = i % 2;
            j = (i - j) + !j;
            edge = &this->edges.list[j];
         }
         tile_id = 1 << (10 + (i / 2)); // set to the "flip H" or "flip V" flag
      }
      if (edge->tile_id == VUIFRAME_TILEID_UNCHANGED)
         continue;
      tile_id |= edge->tile_id;
      tile_id ^= ((u16)edge->flip_h << 10);
      tile_id ^= ((u16)edge->flip_v << 11);
      
      u8 j = i % 2;
      if (i / 2) { // if a horizontal (top/bottom) edge:
         PAINT(tile_id, inner_x[j], j ? y + h - 1 : y, inner_w[j], 1);
      } else { // if a vertical (left/right) edge:
         PAINT(tile_id, j ? x + w - 1 : x, inner_y[j], 1, inner_h[j]);
      }
   }
   
   #undef PAINT
}