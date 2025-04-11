#include "lu/widgets/enum_picker.h"
#include <string.h> // memcpy
#include "gba/defines.h"
#include "decompress.h" // LoadCompressedSpriteSheet
#include "graphics.h" // interface sprites
#include "palette.h"
#include "sound.h" // PlaySE
#include "sprite.h"
#include "constants/songs.h" // SE_SELECT and other sound effect constants

#define sDirectionX data[0]
#define sMoving     data[1]
#define sDistance   data[2]

// Ideally 5, as that's the default `gKeyRepeatContinueDelay`: we move 
// the arrow by one pixel per frame, for as many frames as will pass 
// before the key repeats.
#define VALUE_ARROW_MOVE_DISTANCE 5

static void SpriteCB_EnumPickerArrow(struct Sprite*);

static const struct OamData sOamData_EnumPickerArrow = {
    .y           = 0,
    .affineMode  = ST_OAM_AFFINE_OFF,
    .objMode     = ST_OAM_OBJ_NORMAL,
    .mosaic      = FALSE,
    .bpp         = ST_OAM_4BPP,
    .shape       = SPRITE_SHAPE(8x8),
    .x           = 0,
    .matrixNum   = 0,
    .size        = SPRITE_SIZE(8x8),
    .tileNum     = 0,
    .priority    = 1,
    .paletteNum  = 0,
    .affineParam = 0
};
static const union AnimCmd sSpriteAnim_EnumPickerArrow[] = {
   ANIMCMD_FRAME(0, 0),
   ANIMCMD_END
};
//
static const union AnimCmd* const sSpriteAnimTable_EnumPickerArrow[] = {
   sSpriteAnim_EnumPickerArrow
};

static const struct SpriteTemplate sSpriteTemplate_EnumPickerArrow = {
    .tileTag     = 0,
    .paletteTag  = 0,
    .oam         = &sOamData_EnumPickerArrow,
    .anims       = sSpriteAnimTable_EnumPickerArrow,
    .images      = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback    = SpriteCB_EnumPickerArrow,
};

//

static const u32 sInterfaceSpriteGfx[] = INCBIN_U32("graphics/lu/cgo_menu/interface-sprites.4bpp.lz");
static const u16 sInterfaceSpritePal[] = INCBIN_U16("graphics/lu/cgo_menu/interface-sprites.gbapal");

static void LoadSpriteAssets(u16 tile_tag, u16 palette_tag) {
   const struct CompressedSpriteSheet spritesheet[] = {
      { sInterfaceSpriteGfx, 0x2000, tile_tag },
      {0}
   };
   const struct SpritePalette palette = { sInterfaceSpritePal, palette_tag };
   LoadCompressedSpriteSheet(spritesheet);
   LoadSpritePalette(&palette);
}

//

extern void InitEnumPicker(struct LuEnumPicker* widget, const struct LuEnumPickerInitParams* params) {
   u8 id_l;
   u8 id_r;
   
   LoadSpriteAssets(params->sprite_tags.tile, params->sprite_tags.palette);
   
   widget->base_pos.x = params->base_pos.x;
   widget->base_pos.y = params->base_pos.y;
   
   //
   // Create sprites.
   //
   
   struct SpriteTemplate tmpl;
   memcpy(&tmpl, &sSpriteTemplate_EnumPickerArrow, sizeof(tmpl));
   tmpl.tileTag    = params->sprite_tags.tile;
   tmpl.paletteTag = params->sprite_tags.palette;
   
   widget->sprite_ids.arrow_l = id_l = CreateSprite(&tmpl, 0, 0, 0);
   widget->sprite_ids.arrow_r = id_r = CreateSprite(&tmpl, 0, 0, 0);
   
   gSprites[id_l].invisible = gSprites[id_r].invisible = TRUE;
   
   gSprites[id_r].hFlip = TRUE;
   
   gSprites[id_l].sDirectionX = -1;
   gSprites[id_r].sDirectionX =  1;
   //
   gSprites[id_l].sMoving   = 0;
   gSprites[id_l].sDistance = 0;
   gSprites[id_r].sMoving   = 0;
   gSprites[id_r].sDistance = 0;
   
   gSprites[id_l].x = widget->base_pos.x;
   gSprites[id_r].x = widget->base_pos.x + params->width;
   SetEnumPickerRow(widget, 0);
}

static void SpriteCB_EnumPickerArrow(struct Sprite* sprite) {
   if (sprite->sMoving) {
      sprite->x2 += sprite->sDirectionX;
      if (++sprite->sDistance > VALUE_ARROW_MOVE_DISTANCE) {
         sprite->sMoving   = 0;
         sprite->x2        = 0;
         sprite->sDistance = 0;
      }
   }
}

#define Y_OFFSET_PX 6 // to center-align with text

extern void SetEnumPickerRow(struct LuEnumPicker* widget, u8 row) {
   u8 y = widget->base_pos.y + (row * TILE_HEIGHT * 2) + Y_OFFSET_PX;
   //
   // Sprites are positioned by their centerpoints. We need to adjust 
   // for that, since the above computation is for the top edge.
   //
   y += 4;
   //
   gSprites[widget->sprite_ids.arrow_l].y = y;
   gSprites[widget->sprite_ids.arrow_r].y = y;
}
extern void SetEnumPickerVisible(struct LuEnumPicker* widget, bool8 visible) {
   gSprites[widget->sprite_ids.arrow_l].invisible = !visible;
   gSprites[widget->sprite_ids.arrow_r].invisible = !visible;
}
extern void OnEnumPickerDecreased(struct LuEnumPicker* widget) {
   gSprites[widget->sprite_ids.arrow_l].sMoving = 1;
   PlaySE(SE_SELECT);
}
extern void OnEnumPickerIncreased(struct LuEnumPicker* widget) {
   gSprites[widget->sprite_ids.arrow_r].sMoving = 1;
   PlaySE(SE_SELECT);
}

#undef sDirectionX
#undef sMoving
#undef sDistance

#undef VALUE_ARROW_MOVE_DISTANCE

extern void DestroyEnumPicker(struct LuEnumPicker* widget) {
   if (widget->sprite_ids.arrow_l != SPRITE_NONE) {
      DestroySprite(&gSprites[widget->sprite_ids.arrow_l]);
      widget->sprite_ids.arrow_l = SPRITE_NONE;
   }
   if (widget->sprite_ids.arrow_r != SPRITE_NONE) {
      DestroySprite(&gSprites[widget->sprite_ids.arrow_r]);
      widget->sprite_ids.arrow_r = SPRITE_NONE;
   }
}