#ifndef GUARD_LU_FIELD_DEBUG_MENU
#define GUARD_LU_FIELD_DEBUG_MENU

#include "gba/types.h"

extern struct FieldDebugMenuState {
   bool8 menu_is_open : 1;
   
   bool8 allow_fast_travel_anywhere    : 1;
   bool8 disable_roamer_movement       : 1;
   bool8 disable_trainer_line_of_sight : 1;
   bool8 disable_wild_encounters       : 1;
   bool8 walk_through_walls            : 1;
} gFieldDebugMenuState;

extern void OpenFieldDebugMenu(void);

#endif