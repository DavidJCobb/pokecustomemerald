#ifndef GUARD_LU_UI_WIDGET_SCROLLBAR_COMMON
#define GUARD_LU_UI_WIDGET_SCROLLBAR_COMMON

#include "gba/types.h"

#define SCROLLBAR_TILE_COUNT 4

struct LuScrollbarColors { // palette indices
   u8 track      : 4;
   u8 thumb      : 4;
   u8 background : 4; // background on either side of the scrollbar, and shown if no scrollbar
};

struct LuScrollbar {
   struct {
      u8 bg_layer;
      u8 palette_id;
      struct LuScrollbarColors colors;
      struct {
         u8 x; // in tiles
         u8 y; // in tiles
      } pos;
      u8  length; // in tiles
      u16 first_tile_id;
      
      void* tile_buffer; // heap-allocated
   } graphics;
   u8 scroll_pos;
   u8 item_count;
   u8 max_visible_items;
};

#endif