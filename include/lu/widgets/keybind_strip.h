#ifndef GUARD_LU_UI_WIDGET_KEYBIND_STRIP
#define GUARD_LU_UI_WIDGET_KEYBIND_STRIP

#include "gba/defines.h" // DISPLAY_TILE_WIDTH, PLTT_SIZE_4BPP
#include "gba/types.h"

#define KEYBIND_STRIP_TILE_WIDTH  DISPLAY_TILE_WIDTH
#define KEYBIND_STRIP_TILE_HEIGHT 2
#define KEYBIND_STRIP_TILE_COUNT  (KEYBIND_STRIP_TILE_WIDTH * KEYBIND_STRIP_TILE_HEIGHT)

struct LuKeybindStripEntry {
   u16       buttons; // bitmask
   const u8* text;
};

struct LuKeybindStrip {
   const struct LuKeybindStripEntry* entries;
   u8 entry_count;
   u8 enabled_entries; // bitmask
   u8 _window_id;
};

// Used when constructing a keybind strip widget.
struct LuKeybindStripInitParams {
   u8  bg_layer;
   u16 first_tile_id; // i.e. baseBlock for a Window
   u8  palette_id;    // we'll load the palette for you; just tell us where to load it
};

// Call after any InitWindows calls your UI may use, please.
// We use AddWindow to make our window, but InitWindows sets 
// fixed window indices and is likely to overwrite us.
//
// Zero-initializes the keybind strip widget. Write your entry 
// list after.
extern void InitKeybindStrip(struct LuKeybindStrip* widget, const struct LuKeybindStripInitParams*);

extern void SetKeybindStripAllEntriesEnabled(struct LuKeybindStrip*, bool8 enabled);
extern void SetKeybindStripEntryEnabled(struct LuKeybindStrip*, u8 index, bool8 enabled);

extern void RepaintKeybindStrip(const struct LuKeybindStrip*);

// Call before destroying all windows, please. Will destroy 
// the keybind strip's window.
extern void DestroyKeybindStrip(struct LuKeybindStrip*);

#endif