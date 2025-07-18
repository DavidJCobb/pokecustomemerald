#ifndef GUARD_LU_UI_WIDGET_NUM_EDIT_MODAL
#define GUARD_LU_UI_WIDGET_NUM_EDIT_MODAL

#include "gba/types.h"

typedef s32 LuNumEditModalValue;

typedef void(*LuNumEditModalCallback)(bool8 accepted, LuNumEditModalValue value);

#define NUM_EDIT_MODAL_INNER_TILE_WIDTH_MAX 22
#define NUM_EDIT_MODAL_INNER_TILE_HEIGHT    2

struct LuNumEditModal {
   LuNumEditModalCallback callback;
   LuNumEditModalValue    min_value;
   LuNumEditModalValue    max_value;
   LuNumEditModalValue    cur_value;
   u8                     digit_count;
   
   u8 cursor_pos;
   u8 cursor_sprite_id;
   u8 window_id;
   struct {
      u8 back   : 4;
      u8 text   : 4;
      u8 shadow : 4;
   } text_colors;
   bool8 active : 1;
   bool8 heap_free_on_destroy : 1;
   
   u8 task_id;
};

// Used when constructing a keybind strip widget.
struct LuNumEditModalInitParams {
   LuNumEditModalValue    min_value;
   LuNumEditModalValue    max_value;
   LuNumEditModalValue    cur_value;
   LuNumEditModalCallback callback;
   bool8 use_task;
   struct {
      u8  bg_layer;
      u16 first_tile_id;
      u8  palette_id;
      u8  x; // in tiles; includes the border
      u8  y; // in tiles; includes the border
   } window;
   struct {
      u16   first_tile_id;
      u8    palette_id;
      bool8 already_loaded;
   } border;
   struct {
      u8 back;
      u8 text;
      u8 shadow;
   } text_colors;
   struct {
      struct {
         u16 tile;
         u16 palette;
      } cursor;
   } sprite_tags;
};

extern void InitNumEditModal(struct LuNumEditModal* widget, const struct LuNumEditModalInitParams*);
extern void HandleNumEditModalInput(struct LuNumEditModal*);
extern void DestroyNumEditModal(struct LuNumEditModal*);

// Spawns the modal data on the heap, and spawns a task to run it.
extern void FireAndForgetNumEditModal(const struct LuNumEditModalInitParams*);

struct LuKeybindStrip;
extern void NumEditModalTakeOverKeybindStrip(struct LuKeybindStrip*);

#endif