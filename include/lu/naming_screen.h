#ifndef GUARD_LU_VUI_NAMINGSCREEN_H
#define GUARD_LU_VUI_NAMINGSCREEN_H

#include "gba/types.h"
struct SpritePalette;
struct SpriteTemplate;
struct SubspriteTable;

enum __attribute__((__packed__)) LuNamingScreenIconType {
   LU_NAMINGSCREEN_ICONTYPE_NONE = 0,
   LU_NAMINGSCREEN_ICONTYPE_OVERWORLD,
   LU_NAMINGSCREEN_ICONTYPE_POKEMON,
   LU_NAMINGSCREEN_ICONTYPE_CUSTOM,
   
   // Presets.
   LU_NAMINGSCREEN_ICONTYPE_PC,
   LU_NAMINGSCREEN_ICONTYPE_PLAYER, // ..._OVERWORLD
};

struct LuNamingScreenIcon {
   enum LuNamingScreenIconType type;
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

struct LuNamingScreenParams {
   void(*callback)(const u8*);
   const u8* initial_value;
   u8 max_length;
   
   bool8 has_gender;
   u8    gender; // pokemon or player gender constant
   struct LuNamingScreenIcon icon;
   
   const u8* title; // optional
};

extern void LuNamingScreen(const struct LuNamingScreenParams*);

#endif