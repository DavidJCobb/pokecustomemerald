#include "lu/widgets/loading_spinner.h"
#include <string.h> // memcpy
#include "gba/defines.h"
#include "decompress.h" // LoadCompressedSpriteSheet
#include "palette.h"
#include "sprite.h"
#include "trig.h"

static void SpriteCB_LoadingSpinner(struct Sprite*);

static const struct OamData sOamData_Spinner = {
    .y           = 0,
    .affineMode  = ST_OAM_AFFINE_NORMAL,
    .objMode     = ST_OAM_OBJ_NORMAL,
    .mosaic      = FALSE,
    .bpp         = ST_OAM_4BPP,
    .shape       = SPRITE_SHAPE(32x32),
    .x           = 0,
    .matrixNum   = 0,
    .size        = SPRITE_SIZE(32x32),
    .tileNum     = 0,
    .priority    = 1,
    .paletteNum  = 0,
    .affineParam = 0
};
static const union AnimCmd sSpriteAnim_Spinner[] = {
   ANIMCMD_FRAME(0, 0),
   ANIMCMD_END
};

static const union AnimCmd* const sSpriteAnimTable_Spinner[] = {
   sSpriteAnim_Spinner
};

static const struct SpriteTemplate sSpriteTemplate_Spinner = {
    .tileTag     = 0,
    .paletteTag  = 0,
    .oam         = &sOamData_Spinner,
    .anims       = sSpriteAnimTable_Spinner,
    .images      = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback    = SpriteCB_LoadingSpinner,
};

//

static const u32 sInterfaceSpriteGfx[] = INCBIN_U32("graphics/lu/spinner/spinner-32-aa.4bpp.lz");

static void LoadSpriteAssets(u16 tile_tag, u16 palette_tag, const u16* palette_data) {
   const struct CompressedSpriteSheet spritesheet[] = {
      { sInterfaceSpriteGfx, 0x2000, tile_tag },
      {0}
   };
   const struct SpritePalette palette = { palette_data, palette_tag };
   LoadCompressedSpriteSheet(spritesheet);
   LoadSpritePalette(&palette);
}

//

#define sAngle data[0]

extern u8 SpawnLoadingSpinner(
   u8         x,
   u8         y,
   u16        tiles_tag,
   u16        palette_tag,
   const u16* palette_data,
   u8         matrix_index,
   bool8      visible
) {
   LoadSpriteAssets(tiles_tag, palette_tag, palette_data);
   
   struct SpriteTemplate tmpl;
   memcpy(&tmpl, &sSpriteTemplate_Spinner, sizeof(tmpl));
   tmpl.tileTag    = tiles_tag;
   tmpl.paletteTag = palette_tag;
   
   u8 sprite_id = CreateSprite(&tmpl, 0, 0, 0);
   
   struct Sprite* sprite = &gSprites[sprite_id];
   sprite->oam.matrixNum = matrix_index;
   sprite->invisible = !visible;
   sprite->x = x;
   sprite->y = y;
   
   return sprite_id;
}
extern void SetLoadingSpinnerPosition(u8 sprite_id, u8 x, u8 y) {
   struct Sprite* sprite = &gSprites[sprite_id];
   sprite->x = x;
   sprite->y = y;
}
extern void SetLoadingSpinnerVisible(u8 sprite_id, bool8 visible) {
   struct Sprite* sprite = &gSprites[sprite_id];
   if (sprite->invisible != !visible) {
      sprite->invisible = !visible;
      sprite->sAngle = 0;
   }
}
extern void DestroyLoadingSpinner(u8 sprite_id) {
   DestroySprite(&gSprites[sprite_id]);
}

static void SpriteCB_LoadingSpinner(struct Sprite* sprite) {
   u16 angle = sprite->sAngle;
   s16 sin   = Sin2(angle) / 16;
   s16 cos   = Cos2(angle) / 16;
   SetOamMatrix(sprite->oam.matrixNum, cos, sin, -sin, cos);
   
   sprite->sAngle += 5;
}

#undef sAngle