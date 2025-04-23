#ifndef GUARD_LU_UI_WIDGET_LOADING_SPINNER
#define GUARD_LU_UI_WIDGET_LOADING_SPINNER

#include "gba/types.h"

// returns sprite ID
extern u8 SpawnLoadingSpinner(
   u8         x,
   u8         y,
   u16        tiles_tag,
   u16        palette_tag,
   const u16* palette_data,
   u8         matrix_index,
   bool8      visible
);
extern void SetLoadingSpinnerPosition(u8 spriteId, u8 x, u8 y);
extern void SetLoadingSpinnerVisible(u8 spriteId, bool8 visible);
extern void DestroyLoadingSpinner(u8 spriteId);

#endif