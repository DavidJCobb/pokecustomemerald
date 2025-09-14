#ifndef GUARD_MENU_SHORTSTRINGENTRY_PARAMS
#define GUARD_MENU_SHORTSTRINGENTRY_PARAMS

#include "gba/types.h"
struct SpritePalette;
struct SpriteTemplate;
struct SubspriteTable;

enum __attribute__((__packed__)) ShortStringEntryMenuIconType {
   SHORTSTRINGENTRY_ICONTYPE_NONE = 0,
   SHORTSTRINGENTRY_ICONTYPE_OVERWORLD,
   SHORTSTRINGENTRY_ICONTYPE_POKEMON,
   SHORTSTRINGENTRY_ICONTYPE_CUSTOM,
   
   // Presets.
   SHORTSTRINGENTRY_ICONTYPE_PC,
   SHORTSTRINGENTRY_ICONTYPE_PLAYER, // ..._OVERWORLD
};

struct ShortStringEntryMenuIcon {
   enum ShortStringEntryMenuIconType type;
   union {
      struct {
         const struct SpritePalette*  palette;
         const struct SpriteTemplate* template;
         const struct SubspriteTable* subsprites;
         s8 offset_x : 4;
         s8 offset_y : 4;
      } custom;
      struct {
         u16 id;
      } overworld;
      struct {
         u16 species;
         u32 personality;
      } pokemon;
   };
};

#endif