#include "menus/short_string_entry/cursors.h"
#include "menus/short_string_entry/gfx_tags.h"
#include "menus/short_string_entry/state.h"
#include "lu/c.h"
#include "lu/macros/ARRAY_COUNT.h"
#include "decompress.h"
#include "sprite.h"
#include "constants/rgb.h"

static const struct OamData sOam_8x8;

static const u32 sCharsetCursorGfx[] = INCBIN_U32("graphics/lu/short_string_entry_menu/charset-cursor-subsprites.4bpp.lz");
static const u16 sCursorPal[] = INCBIN_U16("graphics/lu/short_string_entry_menu/cursors.gbapal");

static const struct SpritePalette sSpritePalettes[] = {
    { sCursorPal, SPRITE_PAL_TAG_CURSORS },
    {}
};
static const struct CompressedSpriteSheet sCharsetCursorSpriteSheet = {
   sCharsetCursorGfx, 33 * TILE_SIZE_4BPP, SPRITE_GFX_TAG_CHARSET_CURSORS
};

// Helper for defining a rectangular button consisting of four subsprites:
//  - upper left  (32x16)
//  - lower left  (32x 8)
//  - upper right (16x16)
//  - lower right (16x 8)

#define CHARSET_CURSOR_SUBSPRITES(_width) \
   {                                      \
      .x          = 0,                    \
      .y          = 0,                    \
      .shape      = SPRITE_SHAPE(32x16),  \
      .size       = SPRITE_SIZE(32x16),   \
      .tileOffset = 0,                    \
      .priority   = 3,                    \
   },                                     \
   {                                      \
      .x          =  0,                   \
      .y          = 16,                   \
      .shape      = SPRITE_SHAPE(32x8),   \
      .size       = SPRITE_SIZE(32x8),    \
      .tileOffset = 8,                    \
      .priority   = 3,                    \
   },                                     \
   {                                      \
      .x          = ((_width) - 16),      \
      .y          =  0,                   \
      .shape      = SPRITE_SHAPE(16x16),  \
      .size       = SPRITE_SIZE(16x16),   \
      .tileOffset = 12,                   \
      .priority   = 3,                    \
   },                                     \
   {                                      \
      .x          = ((_width) - 16),      \
      .y          = 16,                   \
      .shape      = SPRITE_SHAPE(16x8),   \
      .size       = SPRITE_SIZE(16x8),    \
      .tileOffset = 16,                   \
      .priority   = 3,                    \
   },

static const struct Subsprite sSubsprites_CharsetCursor_Upper[] = {
   CHARSET_CURSOR_SUBSPRITES(37)
};
static const struct Subsprite sSubsprites_CharsetCursor_Lower[] = {
   CHARSET_CURSOR_SUBSPRITES(36)
};
static const struct Subsprite sSubsprites_CharsetCursor_Symbol[] = {
   CHARSET_CURSOR_SUBSPRITES(43)
};
static const struct Subsprite sSubsprites_CharsetCursor_AccentUpper[] = {
   CHARSET_CURSOR_SUBSPRITES(42)
};
static const struct Subsprite sSubsprites_CharsetCursor_AccentLower[] = {
   CHARSET_CURSOR_SUBSPRITES(40)
};

#undef CHARSET_CURSOR_SUBSPRITES

#define MAKE_SUBSPRITE_TABLE(Name) \
   static const struct SubspriteTable sSubspriteTable_CharsetCursor_##Name[] = {       \
      {ARRAY_COUNT(sSubsprites_CharsetCursor_##Name), sSubsprites_CharsetCursor_##Name} \
   };
MAKE_SUBSPRITE_TABLE(Upper);
MAKE_SUBSPRITE_TABLE(Lower);
MAKE_SUBSPRITE_TABLE(Symbol);
MAKE_SUBSPRITE_TABLE(AccentUpper);
MAKE_SUBSPRITE_TABLE(AccentLower);
#undef MAKE_SUBSPRITE_TABLE

static const struct SpriteTemplate sCharsetCursorSpriteTemplate = {
    .tileTag     = SPRITE_GFX_TAG_CHARSET_CURSORS,
    .paletteTag  = SPRITE_PAL_TAG_CURSORS,
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

extern void ShortStringEntryMenu_SetUpCursors(struct ShortStringEntryMenuState* state) {
   LoadCompressedSpriteSheet(&sCharsetCursorSpriteSheet);
   LoadSpritePalettes(sSpritePalettes);
   
   const u8 x_coords[5] = {
       19,
       57,
       94,
      138,
      181,
   };
   
   #define MAKE_SPRITE(_index, _name) \
      { \
         u8 id = CreateSprite(&sCharsetCursorSpriteTemplate, x_coords[_index], 141, 1); \
         state->sprite_ids.cursor_charset_button_sprites[_index] = id; \
         auto sprite = &gSprites[id]; \
         sprite->invisible = TRUE; \
         SetSubspriteTables(sprite, sSubspriteTable_CharsetCursor_##_name); \
      }
   MAKE_SPRITE(0, Upper)
   MAKE_SPRITE(1, Lower)
   MAKE_SPRITE(2, Symbol)
   MAKE_SPRITE(3, AccentUpper)
   MAKE_SPRITE(4, AccentLower)
   #undef MAKE_SPRITE
}

static void HideCharsetCursors(struct ShortStringEntryMenuState* state) {
   for(int i = 0; i < 5; ++i) {
      u8 id = state->sprite_ids.cursor_charset_button_sprites[i];
      if (id != SPRITE_NONE)
         gSprites[id].invisible = TRUE;
   }
}

extern void ShortStringEntryMenu_UpdateCursors(struct ShortStringEntryMenuState* state) {
   struct Sprite* cursor_mb = NULL;
   
   auto target = state->vui.context.focused;
   if (!target) {
      if (cursor_mb)
         cursor_mb->invisible = TRUE;
      HideCharsetCursors(state);
      return;
   }
   if (target == (const VUIWidget*)&state->vui.widgets.keyboard) {
      if (cursor_mb)
         cursor_mb->invisible = TRUE;
      HideCharsetCursors(state);
   } else if (target == (const VUIWidget*)&state->vui.widgets.button_ok) {
      if (cursor_mb) {
         cursor_mb->invisible = FALSE;
         cursor_mb->x = 172;
         cursor_mb->y =  40;
      }
      HideCharsetCursors(state);
   } else if (target == (const VUIWidget*)&state->vui.widgets.button_backspace) {
      if (cursor_mb) {
         cursor_mb->invisible = FALSE;
         cursor_mb->x = 172;
         cursor_mb->y =  80;
      }
      HideCharsetCursors(state);
   } else {
      if (cursor_mb)
         cursor_mb->invisible = TRUE;
      
      //
      // Check if a charset button has focus.
      //
      #define LIST state->vui.widgets.charset_buttons.list
      for(int i = 0; i < ARRAY_COUNT(LIST); ++i) {
         auto widget = &LIST[i];
         if (target == (const VUIWidget*)widget) {
            for(int j = 0; j < 5; ++j) {
               u8 id = state->sprite_ids.cursor_charset_button_sprites[j];
               if (id != SPRITE_NONE)
                  gSprites[id].invisible = (i != j);
            }
            return;
         }
      }
      #undef LIST
      HideCharsetCursors(state);
   }
}