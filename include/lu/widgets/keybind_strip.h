#ifndef GUARD_LU_UI_WIDGET_KEYBIND_STRIP
#define GUARD_LU_UI_WIDGET_KEYBIND_STRIP

#include "gba/defines.h" // DISPLAY_TILE_WIDTH, PLTT_SIZE_4BPP
#include "gba/types.h"

#define KEYBIND_STRIP_TILE_WIDTH  DISPLAY_TILE_WIDTH
#define KEYBIND_STRIP_TILE_HEIGHT 2

#define KEYBIND_STRIP_ENTRY_COUNT 6

struct LuKeybindStripEntry {
   u16       buttons; // bitmask
   bool8     show;
   const u8* text;
};

struct LuKeybindStrip {
   struct {
      u8 bg_layer;
      u8 palette_id; // we'll load the palette for you; just tell us where to load it
      u8 first_tile_id;
   } config;
   
   // Update these as needed and then call the repaint function.
   // Please ensure that these are either valid or zeroed out 
   // before the first paint.
   LuKeybindStripEntry entries[KEYBIND_STRIP_ENTRY_COUNT];
   
   struct {
      u16 window_id; // we create and destroy this for you
   } state;
};

// Call after any InitWindows calls your UI may use, please.
// We use AddWindow to make our window, but InitWindows sets 
// fixed window indices and is likely to overwrite us.
//
// Please zero-initialize your keybind strip before calling 
// this (i.e. `myStrip = { 0 };`).
void InitKeybindStrip(struct LuKeybindStrip*);

void RepaintKeybindStrip(struct LuKeybindStrip*);

// Call before destroying all windows, please.
void DestroyKeybindStrip(struct LuKeybindStrip*);

#endif