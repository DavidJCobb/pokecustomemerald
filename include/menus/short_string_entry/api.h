#ifndef GUARD_MENU_SHORTSTRINGENTRY_API
#define GUARD_MENU_SHORTSTRINGENTRY_API
#include "gba/types.h"
#include "lu/common_typedefs/maincallback.h"

extern void RenamePCBox(MainCallback, u8* boxName);

extern void RequestPlayerName(MainCallback);

#endif