#ifndef GUARD_LU_VUI_TILEBUTTON_H
#define GUARD_LU_VUI_TILEBUTTON_H

#include "lu/vui/vui-widget.h"

struct VUITileButton_Callbacks {
   void(*on_press)(void);
};
struct VUITileButton_TileIDs {
   u16 corner; // char-based tile ID
   u16 edge_h; // char-based tile ID
   u16 edge_v; // char-based tile ID
   u16 fill;   // char-based tile ID
};
struct VUITileButton_LabelParams {
   const u8* text;
   const u8* button;
   //
   VUITextColors colors;
   u8 y_text;   // in pixels
   u8 y_button; // in pixels
};

typedef struct VUITileButton_InitParams {
   VUIGridArea grid;
   
   struct VUITileButton_Callbacks callbacks;
   
   u8 bg      : 2;
   u8 palette : 4;
   struct VUITileButton_LabelParams labels;
   VUIGridArea tile_area;
   struct {
      struct VUITileButton_TileIDs normal;
      struct VUITileButton_TileIDs focused;
      u16 first_window_tile_id : 9; // char-based tile ID
   } tile_ids;
} VUITileButton_InitParams;

typedef struct VUITileButton {
   VUI_WIDGET_SUBCLASS_HEADER(VUIWidget);
   struct VUITileButton_Callbacks callbacks;
   
   u8 bg;
   u8 palette;
   struct VUITileButton_LabelParams labels;
   VUIGridArea tile_area;
   struct {
      struct VUITileButton_TileIDs normal;
      struct VUITileButton_TileIDs focused;
   } tile_ids;
   u8 window_id;
} VUITileButton;

extern void VUITileButton_Construct(VUITileButton*, const VUITileButton_InitParams*);

#endif