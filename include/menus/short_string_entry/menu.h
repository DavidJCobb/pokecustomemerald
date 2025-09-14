#ifndef GUARD_MENU_SHORTSTRINGENTRY
#define GUARD_MENU_SHORTSTRINGENTRY

#include "lu/c-attr.define.h"
#include "./params.h"

struct ShortStringEntryMenuParams {
   void(*callback)(const u8*);
   const u8* initial_value;
   u8 max_length;
   
   bool8 has_gender;
   u8    gender; // pokemon or player gender constant
   struct ShortStringEntryMenuIcon icon;
   
   const u8* title; // optional
};

NON_NULL_PARAMS(1) extern void OpenShortStringEntryMenu(const struct ShortStringEntryMenuParams*);

#include "lu/c-attr.undef.h"
#endif