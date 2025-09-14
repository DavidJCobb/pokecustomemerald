#include "menus/short_string_entry/cursors.h"
#include "menus/short_string_entry/gfx_tags.h"
#include "menus/short_string_entry/state.h"
#include "lu/c.h"
#include "lu/macros/ARRAY_COUNT.h"
#include "gba/isagbprint.h"
#include "decompress.h"
#include "sprite.h"
#include "constants/rgb.h"

static const struct OamData sOam_8x8;

static const u16 sCursorPal[] = INCBIN_U16("graphics/lu/short_string_entry_menu/cursors.gbapal");

static const u32 sCharsetCursorGfx[] = INCBIN_U32("graphics/lu/short_string_entry_menu/charset-cursor-subsprites.4bpp.lz");
enum {
   CHARSETBUTTON_CURSOR_TILE_COUNT = 33,
};

static const u32 sMenuBtnCursorGfx[] = INCBIN_U32("graphics/lu/short_string_entry_menu/menu-button-cursor-gfx.4bpp.lz");
enum {
   MENUBUTTON_CURSOR_W = 48,
   MENUBUTTON_CURSOR_H = 48,
   MENUBUTTON_CURSOR_TILE_COUNT = (MENUBUTTON_CURSOR_W / TILE_WIDTH) * (MENUBUTTON_CURSOR_H / TILE_HEIGHT),
};

static const struct SpritePalette sSpritePalettes[] = {
    { sCursorPal, SPRITE_PAL_TAG_CURSORS },
    {}
};
static const struct CompressedSpriteSheet sCharsetCursorSpriteSheet = {
   sCharsetCursorGfx, CHARSETBUTTON_CURSOR_TILE_COUNT * TILE_SIZE_4BPP, SPRITE_GFX_TAG_CHARSET_CURSORS
};
static const struct CompressedSpriteSheet sMenuBtnCursorSpriteSheet = {
   sMenuBtnCursorGfx, MENUBUTTON_CURSOR_TILE_COUNT * TILE_SIZE_4BPP, SPRITE_GFX_TAG_MENU_BTN_CURSOR
};

static const u8 sCursorParticleGfx[] = INCBIN_U8("graphics/lu/short_string_entry_menu/cursor-particle.4bpp");
static const struct SpriteSheet sCursorSpriteSheet = {
   sCursorParticleGfx, sizeof(sCursorParticleGfx), SPRITE_GFX_TAG_CURSOR_PARTICLE
};

static const struct SpriteTemplate sCharsetCursorSpriteTemplate;
static const struct SpriteTemplate sMenuButtonCursorSpriteTemplate;
static const struct SpriteTemplate sParticleSpriteTemplate;

#if 1 // Charset cursor subsprites and sprite template
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
#endif

#if 1 // Menu-button cursor subsprites and template
   static const struct Subsprite sSubsprites_MenuButtonCursor[] = {
      {
         .x          = 16,
         .y          = 0,
         .shape      = SPRITE_SHAPE(16x16),
         .size       = SPRITE_SIZE(16x16),
         .tileOffset = 0,
         .priority   = 3,
      },
      {
         .x          = 0,
         .y          = 16,
         .shape      = SPRITE_SHAPE(16x16),
         .size       = SPRITE_SIZE(16x16),
         .tileOffset = 4,
         .priority   = 3,
      },
      {
         .x          = 32,
         .y          = 8,
         .shape      = SPRITE_SHAPE(16x32),
         .size       = SPRITE_SIZE(16x32),
         .tileOffset = 8,
         .priority   = 3,
      },
      {
         .x          = 0,
         .y          = 32,
         .shape      = SPRITE_SHAPE(32x16),
         .size       = SPRITE_SIZE(32x16),
         .tileOffset = 16,
         .priority   = 3,
      },
      {
         .x          = 8,
         .y          = 8,
         .shape      = SPRITE_SHAPE(8x8),
         .size       = SPRITE_SIZE(8x8),
         .tileOffset = 24,
         .priority   = 3,
      },
      {
         .x          = 40,
         .y          = 0,
         .shape      = SPRITE_SHAPE(8x8),
         .size       = SPRITE_SIZE(8x8),
         .tileOffset = 25,
         .priority   = 3,
      },
   };
   static const struct SubspriteTable sSubspriteTable_MenuButtonCursor[] = {
      {ARRAY_COUNT(sSubsprites_MenuButtonCursor), sSubsprites_MenuButtonCursor}
   };
   static const struct SpriteTemplate sMenuButtonCursorSpriteTemplate = {
    .tileTag     = SPRITE_GFX_TAG_MENU_BTN_CURSOR,
    .paletteTag  = SPRITE_PAL_TAG_CURSORS,
    .oam         = &sOam_8x8,
    .anims       = gDummySpriteAnimTable,
    .images      = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback    = SpriteCallbackDummy
   };
#endif

extern void ShortStringEntryMenu_SetUpCursors(struct ShortStringEntryMenuState* state) {
   LoadCompressedSpriteSheet(&sCharsetCursorSpriteSheet);
   LoadCompressedSpriteSheet(&sMenuBtnCursorSpriteSheet);
   LoadSpriteSheet(&sCursorSpriteSheet);
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
   
   {
      u8 id = CreateSprite(&sMenuButtonCursorSpriteTemplate, 0, 0, 1);
      state->sprite_ids.cursor_menu_button = id;
      auto sprite = &gSprites[id];
      sprite->invisible = TRUE;
      SetSubspriteTables(sprite, sSubspriteTable_MenuButtonCursor);
   }
   
   for(int i = 0; i < ARRAY_COUNT(state->sprite_ids.cursor_particles); ++i) {
      u8 id = CreateSprite(&sParticleSpriteTemplate, 0, 0, 1);
      state->sprite_ids.cursor_particles[i] = id;
      if (id == SPRITE_NONE)
         continue;
      auto sprite = &gSprites[id];
      sprite->invisible = TRUE;
   }
}

static void HideCharsetCursors(struct ShortStringEntryMenuState* state) {
   for(int i = 0; i < 5; ++i) {
      u8 id = state->sprite_ids.cursor_charset_button_sprites[i];
      if (id != SPRITE_NONE)
         gSprites[id].invisible = TRUE;
   }
}
static void UpdateCursorParticles(
   struct ShortStringEntryMenuState* state,
   u8    parent_sprite_id,
   bool8 is_menu_button
);

extern void ShortStringEntryMenu_UpdateCursors(struct ShortStringEntryMenuState* state) {
   struct Sprite* cursor_mb = NULL;
   if (state->sprite_ids.cursor_menu_button != SPRITE_NONE)
      cursor_mb = &gSprites[state->sprite_ids.cursor_menu_button];
   
   auto target = state->vui.context.focused;
   if (!target) {
      if (cursor_mb)
         cursor_mb->invisible = TRUE;
      HideCharsetCursors(state);
      UpdateCursorParticles(state, SPRITE_NONE, FALSE);
      return;
   }
   if (target == (const VUIWidget*)&state->vui.widgets.keyboard) {
      if (cursor_mb)
         cursor_mb->invisible = TRUE;
      HideCharsetCursors(state);
      UpdateCursorParticles(state, SPRITE_NONE, FALSE);
   } else if (target == (const VUIWidget*)&state->vui.widgets.button_ok) {
      if (cursor_mb) {
         cursor_mb->invisible = FALSE;
         cursor_mb->x = 180;
         cursor_mb->y =  48;
      }
      HideCharsetCursors(state);
      UpdateCursorParticles(state, state->sprite_ids.cursor_menu_button, TRUE);
   } else if (target == (const VUIWidget*)&state->vui.widgets.button_backspace) {
      if (cursor_mb) {
         cursor_mb->invisible = FALSE;
         cursor_mb->x = 180;
         cursor_mb->y =  88;
      }
      HideCharsetCursors(state);
      UpdateCursorParticles(state, state->sprite_ids.cursor_menu_button, TRUE);
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
               if (id == SPRITE_NONE)
                  continue;
               gSprites[id].invisible = (i != j);
               if (i == j)
                  UpdateCursorParticles(state, id, FALSE);
            }
            return;
         }
      }
      #undef LIST
      HideCharsetCursors(state);
      UpdateCursorParticles(state, SPRITE_NONE, FALSE);
   }
}

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

enum {
   PARTICLE_TYPE_CHARSET,
   PARTICLE_TYPE_MENUBUTTON_UPPER,
   PARTICLE_TYPE_MENUBUTTON_LOWER,
};

#define sType        data[0]
#define sTimer       data[1]
#define sVelocityX   data[2] // 256ths of a pixel
#define sVelocityY   data[3] // 256ths of a pixel
#define sSubpixelPos data[4] // low byte X, high byte Y; both signed
#define sForceReset  data[5]

#define PARTICLE_SPEED_SCALE 2
#define PARTICLE_LIFESPAN    7

#include "random.h"
#include "trig.h"

static void CursorParticleSpriteCB(struct Sprite* sprite);
static const struct OamData sParticleOam = {
   .y          = 0,
   .affineMode = ST_OAM_AFFINE_NORMAL,
   .objMode    = ST_OAM_OBJ_NORMAL,
   .bpp        = ST_OAM_4BPP,
   .shape      = SPRITE_SHAPE(8x8),
   .x          = 0,
   .size       = SPRITE_SIZE(8x8),
   .tileNum    = 0,
   .priority   = 0,
   .paletteNum = 0,
};
static const struct SpriteTemplate sParticleSpriteTemplate = {
   .tileTag     = SPRITE_GFX_TAG_CURSOR_PARTICLE,
   .paletteTag  = SPRITE_PAL_TAG_CURSORS,
   .oam         = &sParticleOam,
   .anims       = gDummySpriteAnimTable,
   .images      = NULL,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback    = CursorParticleSpriteCB
};

static void CursorParticleSpriteCB(struct Sprite* sprite) {
   if (sprite->invisible)
      return;
   ++sprite->sTimer;
   if (sprite->sTimer > PARTICLE_LIFESPAN || sprite->sForceReset) {
      sprite->x2 = 0;
      sprite->y2 = 0;
      SetOamMatrixRotationScaling(sprite->oam.matrixNum, 256 / 4, 256 / 4, 0);
      
      // Reset particle velocity.
      u16 variance   = Random();
      u8  variance_x = variance;
      u8  variance_y = variance >> 8;
      switch (sprite->sType) {
         case PARTICLE_TYPE_CHARSET:
            sprite->sVelocityX =  2 << PARTICLE_SPEED_SCALE;
            sprite->sVelocityY = -4 << PARTICLE_SPEED_SCALE;
            variance_x = (variance_x % 32) - 16;
            variance_y = (variance_y % 64) - 32;
            break;
         case PARTICLE_TYPE_MENUBUTTON_UPPER:
            sprite->sVelocityX =  3 << PARTICLE_SPEED_SCALE;
            sprite->sVelocityY = -3 << PARTICLE_SPEED_SCALE;
            variance_x = (variance_x % 48) - 24;
            variance_y = (variance_y % 48) - 24;
            break;
         case PARTICLE_TYPE_MENUBUTTON_LOWER:
            sprite->sVelocityX = -3 << PARTICLE_SPEED_SCALE;
            sprite->sVelocityY =  3 << PARTICLE_SPEED_SCALE;
            variance_x = (variance_x % 48) - 24;
            variance_y = (variance_y % 48) - 24;
            break;
      }
      sprite->sVelocityX += variance_x;
      sprite->sVelocityY += variance_y;
      
      if (sprite->sType == PARTICLE_TYPE_MENUBUTTON_UPPER) {
         if (variance_x > variance_y) {
         }
      } else if (sprite->sType == PARTICLE_TYPE_MENUBUTTON_LOWER) {
         
      }
      
      if (sprite->sForceReset) {
         //
         // Simulate sprite forward per sTimer, and then exit.
         //
         s16 subpx_x = (s8)(sprite->sSubpixelPos & 0xFF);
         s16 subpx_y = (s8)(sprite->sSubpixelPos >> 8);
         subpx_x += sprite->sVelocityX * (sprite->sTimer - 1);
         subpx_y += sprite->sVelocityY * (sprite->sTimer - 1);
         sprite->x2 += subpx_x >> 8;
         sprite->y2 += subpx_y >> 8;
         sprite->sSubpixelPos = (s8)subpx_x | ((u8)subpx_y << 8);
      } else {
         sprite->sTimer = 0;
      }
      sprite->sForceReset = FALSE;
      // fallthrough.
   }
   if (sprite->sTimer == (PARTICLE_LIFESPAN / 2) + (PARTICLE_LIFESPAN % 2)) {
      //
      // Decrease particle size.
      //
      SetOamMatrixRotationScaling(sprite->oam.matrixNum, 256 / 8, 256 / 8, 0);
   }
   //
   // Animate forward by one frame.
   //
   s16 subpx_x = (s8)(sprite->sSubpixelPos & 0xFF);
   s16 subpx_y = (s8)(sprite->sSubpixelPos >> 8);
   subpx_x += sprite->sVelocityX;
   subpx_y += sprite->sVelocityY;
   sprite->x2 += subpx_x >> 8;
   sprite->y2 += subpx_y >> 8;
   sprite->sSubpixelPos = (s8)subpx_x | ((u8)subpx_y << 8);
}

#define PARTICLE_COUNT     6
#define PARTICLES_PER_SIDE (PARTICLE_COUNT / 2)
static void UpdateCursorParticles(
   struct ShortStringEntryMenuState* state,
   u8    parent_sprite_id,
   bool8 is_menu_button
) {
   if (parent_sprite_id == SPRITE_NONE) {
      //
      // No animated cursor. Hide all particles.
      //
      for(int i = 0; i < PARTICLE_COUNT; ++i) {
         u8 id = state->sprite_ids.cursor_particles[i];
         if (id == SPRITE_NONE)
            continue;
         auto sprite = &gSprites[id];
         sprite->invisible = TRUE;
      }
      return;
   }
   
   if (is_menu_button) {
      u16 parent_x = gSprites[parent_sprite_id].x;
      u16 parent_y = gSprites[parent_sprite_id].y;
      for(int i = 0; i < PARTICLE_COUNT; ++i) {
         u8 id = state->sprite_ids.cursor_particles[i];
         if (id == SPRITE_NONE)
            continue;
         auto sprite = &gSprites[id];
         sprite->invisible = FALSE;
         if (i < PARTICLES_PER_SIDE) {
            sprite->sType = PARTICLE_TYPE_MENUBUTTON_UPPER;
            sprite->x     = parent_x + 37;
            sprite->y     = parent_y + 10;
         } else {
            sprite->sType = PARTICLE_TYPE_MENUBUTTON_LOWER;
            sprite->x     = parent_x + 10;
            sprite->y     = parent_y + 37;
         }
         sprite->sTimer       = ((i % PARTICLES_PER_SIDE) == 1) ? 3 : 0;
         sprite->sForceReset  = TRUE;
         sprite->sSubpixelPos = 0;
      }
   } else {
      u16 parent_x = gSprites[parent_sprite_id].x;
      u16 parent_y = gSprites[parent_sprite_id].y;
      for(int i = 0; i < PARTICLES_PER_SIDE; ++i) {
         u8 id = state->sprite_ids.cursor_particles[i];
         if (id == SPRITE_NONE)
            continue;
         auto sprite = &gSprites[id];
         sprite->invisible = FALSE;
         sprite->sType        = PARTICLE_TYPE_CHARSET;
         sprite->sTimer       = (i == 1) ? 3 : 0;
         sprite->sForceReset  = TRUE;
         sprite->sSubpixelPos = 0;
         
         sprite->x = parent_x + 20;
         sprite->y = parent_y;
      }
      //
      // Hide the sprites used for particles cast out the bottom, since they'd be 
      // off-screen.
      //
      for(int i = PARTICLES_PER_SIDE; i < PARTICLE_COUNT; ++i) {
         u8 id = state->sprite_ids.cursor_particles[i];
         if (id == SPRITE_NONE)
            continue;
         auto sprite = &gSprites[id];
         sprite->invisible = TRUE;
      }
   }
   
}