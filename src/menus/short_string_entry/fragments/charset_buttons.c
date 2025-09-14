#include "menus/short_string_entry/fragments/charset_buttons.h"
#include "menus/short_string_entry/gfx_tags.h"
#include "lu/c.h"
#include "lu/macros/ARRAY_COUNT.h"
#include "sprite.h"

#define CHARSET_BUTTON_POS_X  20
#define CHARSET_BUTTON_POS_Y 142

static const struct OamData sOam_8x8;

//
// TODO: Merge all of the buttons into a single image and use subsprites, as was 
//       done in the previous naming-screen design. Sprites have limited sizes; 
//       we can't have e.g. a 48x16 or 64x16 sprite, but that's precisely what 
//       we need. Subsprites work around that limitation.
//

static const u8 sSprite[] = INCBIN_U8("graphics/lu/short_string_entry_menu/charset-buttons.4bpp");

static const u16 sPaletteNormal[] = INCBIN_U16("graphics/lu/short_string_entry_menu/charset-button-normal.gbapal");
static const u16 sPaletteActive[] = INCBIN_U16("graphics/lu/short_string_entry_menu/charset-button-active.gbapal");

static const struct SpritePalette sSpritePalettes[] = {
    { sPaletteNormal, SPRITE_PAL_TAG_CHARSET_BUTTON_NORMAL },
    { sPaletteActive, SPRITE_PAL_TAG_CHARSET_BUTTON_ACTIVE },
    {}
};
static const struct SpriteSheet sSpriteSheets[] = {
   { sSprite, sizeof(sSprite), SPRITE_GFX_TAG_CHARSET_BUTTONS },
   {},
};

// Helper for defining a rectangular button consisting of four subsprites:
//  - upper-left corner and upper edge (32x8px)
//  - lower-left corner and lower edge (32x8px)
//  - upper-right corner (either 8x8px or 16x8px)
//  - lower-right corner (either 8x8px or 16x8px)
#define SPRITE_TILE_WIDTH (208 / TILE_WIDTH)
#define SUBSPRITE_GROUP(_x, _w) \
   {                                    \
      .x          = 0,                  \
      .y          = 0,                  \
      .shape      = SPRITE_SHAPE(32x8), \
      .size       = SPRITE_SIZE(32x8),  \
      .tileOffset = (_x) / TILE_WIDTH,  \
      .priority   = 1,                  \
   },                                   \
   {                                    \
      .x          = 0,                  \
      .y          = 8,                  \
      .shape      = SPRITE_SHAPE(32x8), \
      .size       = SPRITE_SIZE(32x8),  \
      .tileOffset = SPRITE_TILE_WIDTH + (_x) / TILE_WIDTH,  \
      .priority   = 1,                  \
   },                                   \
   {                                    \
      .x          = 32,                 \
      .y          =  0,                 \
      .shape      = (_w) - 32 <= 8 ? SPRITE_SHAPE(8x8) : SPRITE_SHAPE(16x8), \
      .size       = (_w) - 32 <= 8 ? SPRITE_SIZE(8x8)  : SPRITE_SIZE(16x8),  \
      .tileOffset = (_x / TILE_WIDTH) + 4, \
      .priority   = 1,                  \
   },                                   \
   {                                    \
      .x          = 32,                 \
      .y          =  8,                 \
      .shape      = (_w) - 32 <= 8 ? SPRITE_SHAPE(8x8) : SPRITE_SHAPE(16x8), \
      .size       = (_w) - 32 <= 8 ? SPRITE_SIZE(8x8)  : SPRITE_SIZE(16x8),  \
      .tileOffset = SPRITE_TILE_WIDTH + (_x / TILE_WIDTH) + 4, \
      .priority   = 1,                  \
   },
   
static const struct Subsprite sSubsprites_CharsetLabel_Upper[] = {
   SUBSPRITE_GROUP(0, 40)
};
static const struct Subsprite sSubsprites_CharsetLabel_Lower[] = {
   SUBSPRITE_GROUP(40, 40)
};
static const struct Subsprite sSubsprites_CharsetLabel_Symbol[] = {
   SUBSPRITE_GROUP(80, 48)
};
static const struct Subsprite sSubsprites_CharsetLabel_AccentUpper[] = {
   SUBSPRITE_GROUP(128, 40)
};
static const struct Subsprite sSubsprites_CharsetLabel_AccentLower[] = {
   SUBSPRITE_GROUP(168, 38)
};

#undef SPRITE_TILE_WIDTH
#undef SUBSPRITE_GROUP

#define MAKE_SUBSPRITE_TABLE(Name) \
   static const struct SubspriteTable sSubspriteTable_CharsetLabel_##Name[] = {       \
      {ARRAY_COUNT(sSubsprites_CharsetLabel_##Name), sSubsprites_CharsetLabel_##Name} \
   };
MAKE_SUBSPRITE_TABLE(Upper);
MAKE_SUBSPRITE_TABLE(Lower);
MAKE_SUBSPRITE_TABLE(Symbol);
MAKE_SUBSPRITE_TABLE(AccentUpper);
MAKE_SUBSPRITE_TABLE(AccentLower);
#undef MAKE_SUBSPRITE_TABLE

static const struct SpriteTemplate sSpriteTemplate = {
    .tileTag     = SPRITE_GFX_TAG_CHARSET_BUTTONS,
    .paletteTag  = SPRITE_PAL_TAG_CHARSET_BUTTON_NORMAL,
    .oam         = &sOam_8x8,
    .anims       = gDummySpriteAnimTable,
    .images      = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback    = SpriteCallbackDummy
};

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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ShortStringEntryMenu_SetUpCharsetButtons(union ShortStringEntryMenuCharsetButtons* buttons) {
   struct VUISpriteButton_InitParams widget_init_params = {
      .callbacks = {
         .on_press = NULL,
      },
      .grid = {
         .pos  = { 0, 0 },
         .size = { 1, 1 },
      },
   };
   
   LoadSpriteSheets(sSpriteSheets);
   LoadSpritePalettes(sSpritePalettes);
   
   {  // Charset button: Upper
      auto id     = CreateSprite(&sSpriteTemplate, CHARSET_BUTTON_POS_X, CHARSET_BUTTON_POS_Y, 0);
      auto sprite = &gSprites[id];
      auto widget = &buttons->upper;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Upper);
      
      VUISpriteButton_Initialize(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, id);
   }
   {  // Charset button: Lower
      auto id     = CreateSprite(&sSpriteTemplate, CHARSET_BUTTON_POS_X+38, CHARSET_BUTTON_POS_Y, 0);
      auto sprite = &gSprites[id];
      auto widget = &buttons->lower;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Lower);
      
      VUISpriteButton_Initialize(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, id);
   }
   {  // Charset button: Symbol
      auto id     = CreateSprite(&sSpriteTemplate, CHARSET_BUTTON_POS_X+75, CHARSET_BUTTON_POS_Y, 0);
      auto sprite = &gSprites[id];
      auto widget = &buttons->symbol;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Symbol);
      
      VUISpriteButton_Initialize(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, id);
   }
   {  // Charset button: Accented Upper
      auto id     = CreateSprite(&sSpriteTemplate, CHARSET_BUTTON_POS_X+119, CHARSET_BUTTON_POS_Y, 0);
      auto sprite = &gSprites[id];
      auto widget = &buttons->accent_u;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_AccentUpper);
      
      VUISpriteButton_Initialize(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, id);
   }
   {  // Charset button: Accented Lower
      auto id     = CreateSprite(&sSpriteTemplate, CHARSET_BUTTON_POS_X+162, CHARSET_BUTTON_POS_Y, 0);
      auto sprite = &gSprites[id];
      auto widget = &buttons->accent_l;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_AccentLower);
      
      VUISpriteButton_Initialize(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, id);
   }
}

extern void ShortStringEntryMenu_UpdateSelectedCharsetButtonSprite(
   union ShortStringEntryMenuCharsetButtons* buttons,
   enum ShortStringEntryMenu_Charset         selected
) {
   for(int i = 0; i < 5; ++i) {
      auto widget = &buttons->list[i];
      if (widget->sprite_id == SPRITE_NONE)
         continue;
      auto sprite = &gSprites[widget->sprite_id];
      if (!sprite->inUse)
         continue;
      if (i == selected) {
         sprite->oam.paletteNum = IndexOfSpritePaletteTag(SPRITE_PAL_TAG_CHARSET_BUTTON_ACTIVE);
      } else {
         sprite->oam.paletteNum = IndexOfSpritePaletteTag(SPRITE_PAL_TAG_CHARSET_BUTTON_NORMAL);
      }
   }
}