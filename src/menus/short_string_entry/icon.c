#include "menus/short_string_entry/icon.h"
#include "menus/short_string_entry/gfx_tags.h"
#include "menus/short_string_entry/params.h"
#include "lu/c.h"
#include "sprite.h"

enum {
   ICON_BASE_CX = 20, // centerpoint
   ICON_BASE_CY = 16,
   ICON_OFFSET_PKMN_X =  0,
   ICON_OFFSET_PKMN_Y = -4,
   ICON_OFFSET_OW_X   =  0,
   ICON_OFFSET_OW_Y   = -5,
};

static const struct OamData sOam_8x8;

static const struct SpritePalette  sSpritePalette_PCIcon;
static const struct SpriteTemplate sSpriteTemplate_PCIcon;
static const struct SubspriteTable sSubspriteTable_PCIcon[];

#include "global.h" // dependency for some unknown number of the headers below
#include "constants/event_object_movement.h" // ANIM_STD_GO_SOUTH
#include "global.fieldmap.h" // PLAYER_AVATAR_STATE_NORMAL
#include "event_object_movement.h" // CreateObjectGraphicsSprite
#include "field_player_avatar.h" // GetRivalAvatarGraphicsIdByStateIdAndGender
#include "pokemon_icon.h" // CreateMonIcon

// HACK HACK HACK to get the appropriate gender if the icon type is "PLAYER"
#include "menus/short_string_entry/state.h"

u8 ShortStringEntryMenu_ConstructIcon(struct ShortStringEntryMenuIcon* icon) {
   enum {
      PRIORITY = 0,
   };
   
   if (icon->type == SHORTSTRINGENTRY_ICONTYPE_NONE)
      return SPRITE_NONE;
   
   switch (icon->type) { // Handle presets.
      case SHORTSTRINGENTRY_ICONTYPE_PC:
         icon->type = SHORTSTRINGENTRY_ICONTYPE_CUSTOM;
         icon->custom.palette    = &sSpritePalette_PCIcon;
         icon->custom.template   = &sSpriteTemplate_PCIcon;
         icon->custom.subsprites = sSubspriteTable_PCIcon;
         icon->custom.offset_x   = 0;
         icon->custom.offset_y   = 1;
         break;
      case SHORTSTRINGENTRY_ICONTYPE_PLAYER:
         icon->type = SHORTSTRINGENTRY_ICONTYPE_OVERWORLD;
         {
            u8 gender = gShortStringEntryMenuState->gender;
            if (gender == MON_GENDERLESS) {
               gender = gSaveBlock2Ptr->playerGender;
            }
            icon->overworld.id = GetRivalAvatarGraphicsIdByStateIdAndGender(
               PLAYER_AVATAR_STATE_NORMAL,
               gender
            );
         }
         break;
   }
   
   const u8 type = icon->type;
   
   if (type == SHORTSTRINGENTRY_ICONTYPE_CUSTOM) {
      auto params = &icon->custom;
      AGB_ASSERT(params->palette  != NULL);
      AGB_ASSERT(params->template != NULL);
      if (params->palette && params->template) {
         AGB_WARNING(params->palette->tag == params->template->paletteTag);
         AGB_WARNING(params->palette->tag != TAG_NONE && params->template->paletteTag != TAG_NONE);
      }
      
      LoadSpritePalette(params->palette);
      u8 sprite_id = CreateSprite(
         params->template,
         ICON_BASE_CX + params->offset_x,
         ICON_BASE_CY + params->offset_y,
         0
      );
      if (sprite_id != SPRITE_NONE) {
         auto sprite = &gSprites[sprite_id];
         if (params->subsprites)
            SetSubspriteTables(sprite, params->subsprites);
         sprite->oam.priority = PRIORITY;
      }
      return sprite_id;
   }
   
   if (type == SHORTSTRINGENTRY_ICONTYPE_POKEMON) {
      LoadMonIconPalettes();
      u8 sprite_id = CreateMonIcon(
         icon->pokemon.species,
         SpriteCallbackDummy,
         ICON_BASE_CX + ICON_OFFSET_PKMN_X,
         ICON_BASE_CY + ICON_OFFSET_PKMN_Y,
         0,
         icon->pokemon.personality,
         1
      );
      if (sprite_id != SPRITE_NONE) {
         auto sprite = &gSprites[sprite_id];
         sprite->oam.priority = PRIORITY;
      }
      return sprite_id;
   }
   
   if (type == SHORTSTRINGENTRY_ICONTYPE_OVERWORLD) {
      u8 sprite_id = CreateObjectGraphicsSprite(
         icon->overworld.id,
         SpriteCallbackDummy,
         ICON_BASE_CX + ICON_OFFSET_OW_X,
         ICON_BASE_CY + ICON_OFFSET_OW_Y,
         0
      );
      if (sprite_id != SPRITE_NONE) {
         auto sprite = &gSprites[sprite_id];
         sprite->oam.priority = PRIORITY;
         StartSpriteAnim(sprite, ANIM_STD_GO_SOUTH);
      }
      return sprite_id;
   }
   
   AGB_WARNING(0 && "Unhandled ShortStringEntryMenuIconType type!");
   return SPRITE_NONE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static const u8 sPCIconOff_Gfx[] = INCBIN_U8("graphics/naming_screen/pc_icon_off.4bpp");
static const u8 sPCIconOn_Gfx[]  = INCBIN_U8("graphics/naming_screen/pc_icon_on.4bpp");
//
static const union AnimCmd sAnim_PCIcon[] = {
   ANIMCMD_FRAME(0, 2),
   ANIMCMD_FRAME(1, 2),
   ANIMCMD_JUMP(0)
};
static const union AnimCmd* const sAnims_PCIcon[] = {
   sAnim_PCIcon
};
static const struct SpriteFrameImage sImageTable_PCIcon[] = {
   { sPCIconOff_Gfx, sizeof(sPCIconOff_Gfx) },
   { sPCIconOn_Gfx,  sizeof(sPCIconOn_Gfx)  },
};
static const struct SpriteTemplate sSpriteTemplate_PCIcon = {
   .tileTag     = TAG_NONE,
   .paletteTag  = SPRITE_PAL_TAG_CUSTOM_ICON,
   .oam         = &sOam_8x8,
   .anims       = sAnims_PCIcon,
   .images      = sImageTable_PCIcon,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback    = SpriteCallbackDummy
};

/*
[0_]    16x24
[1+] <--Origin
[2_]
*/
static const struct Subsprite sSubsprites_PCIcon[] = {
   {
      .x          = -8,
      .y          = -12,
      .shape      = SPRITE_SHAPE(16x8),
      .size       = SPRITE_SIZE(16x8),
      .tileOffset = 0,
      .priority   = 3
   },
   {
      .x          = -8,
      .y          = -4,
      .shape      = SPRITE_SHAPE(16x8),
      .size       = SPRITE_SIZE(16x8),
      .tileOffset = 2,
      .priority   = 3
   },
   {
      .x          = -8,
      .y          =  4,
      .shape      = SPRITE_SHAPE(16x8),
      .size       = SPRITE_SIZE(16x8),
      .tileOffset = 4,
      .priority   = 3
   }
};
static const struct SubspriteTable sSubspriteTable_PCIcon[] = {
   { ARRAY_COUNT(sSubsprites_PCIcon), sSubsprites_PCIcon }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static const struct OamData sOam_8x8 = {
   .y          = 0,
   .affineMode = ST_OAM_AFFINE_OFF,
   .objMode    = ST_OAM_OBJ_NORMAL,
   .bpp        = ST_OAM_4BPP,
   .shape      = SPRITE_SHAPE(8x8),
   .x          = 0,
   .size       = SPRITE_SIZE(8x8),
   .tileNum    = 0,
   .priority   = 0,
   .paletteNum = 0,
};
