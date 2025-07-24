#ifndef GUARD_LU_VUI_TILEBUTTON_H
#define GUARD_LU_VUI_TILEBUTTON_H

#include "lu/vui/vui-widget.h"

struct VUITileButton_Callbacks {
   void(*on_press)(void);
};
struct VUITileButton_GraphicsParams {
   u8          bg      : 2;
   u8          palette : 4;
   VUISize     size;
   const u8*   data; // eight 4bpp tiles
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
   
   struct VUITileButton_Callbacks      callbacks;
   struct VUITileButton_LabelParams    labels;
   struct VUITileButton_GraphicsParams tiles;
   VUIPos screen_pos;
   u16    first_window_tile_id : 10; // char-based tile ID
} VUITileButton_InitParams;

typedef struct VUITileButton {
   VUI_WIDGET_SUBCLASS_HEADER(VUIWidget);
   struct VUITileButton_Callbacks      callbacks;
   struct VUITileButton_LabelParams    labels;
   struct VUITileButton_GraphicsParams tiles;
   VUIPos screen_pos; // measured in tiles
   u8     window_id;
} VUITileButton;

extern void VUITileButton_Construct(VUITileButton*, const VUITileButton_InitParams*);
extern void VUITileButton_Repaint(VUITileButton*, bool8 is_focused);

#endif