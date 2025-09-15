#ifndef GUARD_MENU_SHORTSTRINGENTRY_API
#define GUARD_MENU_SHORTSTRINGENTRY_API
#include "gba/types.h"
#include "lu/common_typedefs/maincallback.h"

extern void ShortStringEntryMenu_RenamePCBox(MainCallback, u8* boxName);

extern void ShortStringEntryMenu_RenamePlayer(MainCallback);

extern void ShortStringEntryMenu_WaldasPassword(MainCallback, u8* string);

#endif