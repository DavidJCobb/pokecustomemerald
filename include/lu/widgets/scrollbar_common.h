#ifndef GUARD_LU_UI_WIDGET_SCROLLBAR_COMMON
#define GUARD_LU_UI_WIDGET_SCROLLBAR_COMMON

#include "gba/types.h"

#define SCROLLBAR_TILE_COUNT 4

struct LuScrollbarColors { // palette indices
   u8 track      : 4;
   u8 thumb      : 4;
   u8 background : 4; // background on either side of the scrollbar, and shown if no scrollbar
};

struct LuScrollbarGraphicsParams {
   u8  bg_layer    : 2;
   u8  palette_id  : 4;
   u8  color_track : 4;
   u8  color_thumb : 4;
   u8  color_blank : 4; // background around the scrollbar
   u8  pos_x       : 5; // in tiles
   u8  pos_y       : 5; // in tiles
   u8  length      : 5; // in tiles
   u16 first_tile_id;
};

struct LuScrollbar {
   struct {
      struct LuScrollbarGraphicsParams info;
      void* tile_buffer; // heap-allocated
   } graphics;
   u16 scroll_pos;
   u16 item_count;
   u16 max_visible_items;
};

#endif