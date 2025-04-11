#ifndef GUARD_LU_UI_WIDGET_ENUM_PICKER
#define GUARD_LU_UI_WIDGET_ENUM_PICKER

#include "gba/types.h"

#define KEYBIND_STRIP_TILE_HEIGHT 2

struct LuEnumPicker {
   struct {
      u8 x;
      u8 y;
   } base_pos;
   struct {
      u8 arrow_l;
      u8 arrow_r;
   } sprite_ids;
};

// Used when constructing a keybind strip widget.
struct LuEnumPickerInitParams {
   struct {
      u8 x;
      u8 y;
   } base_pos;
   
   // Tags uniquely identify resources used by a sprite. If two sprites 
   // with the same tile tag load, then they will share sprite tiles in
   // VRAM; and thus also for the palette tag.
   struct {
      u16 tile;
      u16 palette;
   } sprite_tags;
   
   u8 width;
};

extern void InitEnumPicker(struct LuEnumPicker* widget, const struct LuEnumPickerInitParams*);

extern void SetEnumPickerRow(struct LuEnumPicker*, u8 row);
extern void SetEnumPickerVisible(struct LuEnumPicker*, bool8 visible);
extern void OnEnumPickerDecreased(struct LuEnumPicker*);
extern void OnEnumPickerIncreased(struct LuEnumPicker*);

extern void DestroyEnumPicker(struct LuEnumPicker*);

#endif