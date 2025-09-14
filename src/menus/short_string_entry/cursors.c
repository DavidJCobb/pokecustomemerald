#include "menus/short_string_entry/cursors.h"
#include "menus/short_string_entry/gfx_tags.h"
#include "menus/short_string_entry/state.h"
#include "lu/c.h"
#include "lu/macros/ARRAY_COUNT.h"
#include "sprite.h"
#include "constants/rgb.h"

static const struct OamData sOam_8x8;

static const u32 sCharsetCursorGfx[] = INCBIN_U32("graphics/lu/short_string_entry_menu/charset-button-cursor-subsprites.4bpp.lz");
static const u16 sCursorPal[] = INCBIN_U16("graphics/lu/short_string_entry_menu/cursors.gbapal");

static const struct SpritePalette sSpritePalettes[] = {
    { sCursorPal, SPRITE_PAL_TAG_CURSORS },
    {}
};
static const struct SpriteSheet sSpriteSheets[] = {
   { sCharsetCursorGfx, sizeof(sCharsetCursorGfx), SPRITE_GFX_TAG_CHARSET_CURSORS },
   {},
};

// Helper for defining a rectangular button consisting of four subsprites:
//  - lefthand edge (32x32)
//  - upper right   ( 8x16 or 16x16)
//  - lower right   ( 8x 8 or 16x 8)

#define SUBSPRITES_LEFTHAND \
   {                                     \
      .x          = 0,                   \
      .y          = 0,                   \
      .shape      = SPRITE_SHAPE(32x16), \
      .size       = SPRITE_SIZE(32x16),  \
      .tileOffset = 0,                   \
      .priority   = 1,                   \
   },                                    \
   {                                     \
      .x          =  0,                  \
      .y          = 16,                  \
      .shape      = SPRITE_SHAPE(32x8), \
      .size       = SPRITE_SIZE(32x8),  \
      .tileOffset = 8,                   \
      .priority   = 1,                   \
   }

#define SUBSPRITES_WIDTH_8(_tile_offset) \
   {                                     \
      .x          = 32,                  \
      .y          =  0,                  \
      .shape      = SPRITE_SHAPE(8x16),  \
      .size       = SPRITE_SIZE(8x16),   \
      .tileOffset = _tile_offset,        \
      .priority   = 1,                   \
   },                                    \
   {                                     \
      .x          = 32,                  \
      .y          = 16,                  \
      .shape      = SPRITE_SHAPE(8x8),   \
      .size       = SPRITE_SIZE(8x8),    \
      .tileOffset = _tile_offset + 2,    \
      .priority   = 1,                   \
   },

#define SUBSPRITES_WIDTH_16(_tile_offset) \
   {                                     \
      .x          = 32,                  \
      .y          =  0,                  \
      .shape      = SPRITE_SHAPE(16x16), \
      .size       = SPRITE_SIZE(16x16),  \
      .tileOffset = _tile_offset,        \
      .priority   = 1,                   \
   },                                    \
   {                                     \
      .x          = 32,                  \
      .y          = 16,                  \
      .shape      = SPRITE_SHAPE(16x8),  \
      .size       = SPRITE_SIZE(16x8),   \
      .tileOffset = _tile_offset + 4,    \
      .priority   = 1,                   \
   },

static const struct Subsprite sSubsprites_CharsetCursor_Upper[] = {
   SUBSPRITES_LEFTHAND,
   SUBSPRITES_WIDTH_8(9)
};
static const struct Subsprite sSubsprites_CharsetCursor_Lower[] = {
   SUBSPRITES_LEFTHAND,
   SUBSPRITES_WIDTH_8(12)
};
static const struct Subsprite sSubsprites_CharsetCursor_Symbol[] = {
   SUBSPRITES_LEFTHAND,
   SUBSPRITES_WIDTH_16(15)
};
static const struct Subsprite sSubsprites_CharsetCursor_AccentUpper[] = {
   SUBSPRITES_LEFTHAND,
   SUBSPRITES_WIDTH_16(21)
};
static const struct Subsprite sSubsprites_CharsetCursor_AccentLower[] = {
   SUBSPRITES_LEFTHAND,
   SUBSPRITES_WIDTH_8(27)
};

#undef SUBSPRITES_LEFTHAND
#undef SUBSPRITES_WIDTH_8
#undef SUBSPRITES_WIDTH_16

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
   #define MAKE_SPRITE(_index, _name) \
      { \
         u8 id = CreateSprite(&sCharsetCursorSpriteTemplate, 37, 17, 0); \
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