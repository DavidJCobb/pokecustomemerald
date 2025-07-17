#include "lu/widgets/textedit_value.h"
#include "constants/characters.h"
#include "gba/gba.h"
#include "gba/isagbprint.h"
#include "global.h"
#include "graphics.h"
#include "menu.h" // AddTextPrinterParameterized3
#include "palette.h"
#include "sprite.h"
#include "string_util.h"
#include "window.h"

static const struct SpriteTemplate sSpriteTemplate_Underscore;
static void LoadSpriteAssets(u16 tile_tag, u16 palette_tag);

extern void InitTexteditValueWidget(
   struct LuTexteditValueWidget* widget,
   const struct LuTexteditValueWidgetInitParams* params
) {
   widget->max_length = params->max_length;
   
   for(u8 i = 0; i < sizeof(widget->underscore_sprite_ids); ++i) {
      widget->underscore_sprite_ids[i] = SPRITE_NONE;
   }
   LoadSpriteAssets(params->sprite_tags.tile, params->sprite_tags.palette);
   
   u8 screen_x = 0;
   u8 screen_y = 0;
   
   widget->owns_window = params->use_own_window;
   if (params->use_own_window) {
      const struct WindowTemplate tmpl = {
         .bg          = params->window_to_create.bg_layer,
         .tilemapLeft = params->window_to_create.x,
         .tilemapTop  = params->window_to_create.y,
         .width       = params->max_length * 2,
         .height      = 2,
         .paletteNum  = params->color.palette_id,
         .baseBlock   = params->window_to_create.first_tile_id
      };
      u8 window_id = AddWindow(&tmpl);
      AGB_ASSERT(window_id != WINDOW_NONE); // Assert that window creation was successful.
      
      widget->window = window_id;
      widget->win_x  = 0;
      widget->win_y  = 0;
      
      PutWindowTilemap(window_id);
      FillWindowPixelBuffer(window_id, PIXEL_FILL(params->color.bg));
      CopyWindowToVram(window_id, COPYWIN_FULL);
      
      screen_x = params->window_to_create.x * TILE_WIDTH;
      screen_y = params->window_to_create.y * TILE_HEIGHT;
   } else {
      widget->window = params->window_to_share.window_id;
      widget->win_x  = params->window_to_share.x;
      widget->win_x  = params->window_to_share.y;
      
      screen_x  = gWindows[widget->window].window.tilemapLeft * TILE_WIDTH;
      screen_y  = gWindows[widget->window].window.tilemapTop  * TILE_HEIGHT;
      screen_x += params->window_to_share.x;
      screen_y += params->window_to_share.y;
   }
   
   widget->color.palette_id = params->color.palette_id;
   widget->color.bg         = params->color.bg;
   widget->color.text       = params->color.text;
   widget->color.shadow     = params->color.shadow;
   
   {
      u8 sprite_x = screen_x;
      
      struct SpriteTemplate tmpl;
      memcpy(&tmpl, &sSpriteTemplate_Underscore, sizeof(tmpl));
      tmpl.tileTag    = params->sprite_tags.tile;
      tmpl.paletteTag = params->sprite_tags.palette;
      
      for(u8 i = 0; i < params->max_length; ++i, (sprite_x += 8)) {
         u8 sprite_id = CreateSprite(&tmpl, sprite_x + 3, screen_y + 1, 0);
         widget->underscore_sprite_ids[i] = sprite_id;
         gSprites[sprite_id].oam.priority = 3;
         gSprites[sprite_id].data[0]      = i;
         gSprites[sprite_id].invisible    = TRUE;
      }
   }
}

extern void ShowTexteditValueWidget(struct LuTexteditValueWidget* widget) {
   for(u8 i = 0; i < sizeof(widget->underscore_sprite_ids); ++i) {
      u8 sprite_id = widget->underscore_sprite_ids[i];
      if (sprite_id == SPRITE_NONE)
         continue;
      gSprites[sprite_id].invisible = FALSE;
      if (i == widget->cursor_pos) {
         gSprites[sprite_id].data[4] = TRUE;
      }
   }
}

static void ClearBeforePaint(struct LuTexteditValueWidget* widget) {
   if (widget->owns_window) {
      FillWindowPixelBuffer(widget->window, PIXEL_FILL(widget->color.bg));
   } else {
      FillWindowPixelRect(
         widget->window,
         PIXEL_FILL(widget->color.bg),
         widget->win_x,
         widget->win_y,
         widget->max_length * 8,
         16
      );
   }
}

extern void PaintTexteditValueNumber(
   struct LuTexteditValueWidget* widget,
   u32 number,
   enum StringConvertMode mode
) {
   u8 max_length = widget->max_length;
   if (max_length > 10) {
      // TODO: Warn: GF functions break past this threshold
      max_length = 10;
   }
   
   u8  buffer[11];
   u8* end = ConvertUIntToDecimalStringN(buffer, number, mode, max_length);
   u8  digits = end - &buffer[0];
   
   u8 draw_buf[2];
   draw_buf[1] = EOS;
   
   u8 x = widget->win_x;
   if (digits > widget->max_length) {
      u8 diff = digits - widget->max_length;
      
      draw_buf[0] = CHAR_0;
      for(u8 i = 0; i < diff; ++i) {
         AddTextPrinterParameterized(widget->window, FONT_NORMAL, draw_buf, x, 1, TEXT_SKIP_DRAW, NULL);
         x += 8;
      }
   }
   for(u8 i = 0; i < max_length; ++i) {
      draw_buf[0] = buffer[i];
      AddTextPrinterParameterized(widget->window, FONT_NORMAL, draw_buf, x, 1, TEXT_SKIP_DRAW, NULL);
      x += 8;
   }
}
extern void PaintTexteditValueString(struct LuTexteditValueWidget* widget, const u8* string) {
   ClearBeforePaint(widget);
   
   u8 buffer[2];
   buffer[1] = EOS;
   
   u8 max_chars = widget->max_length;
   u8 x         = widget->win_x;
   for(u8 i = 0; i < max_chars; ++i) {
      buffer[0] = string[i];
      AddTextPrinterParameterized(widget->window, FONT_NORMAL, buffer, x, 1, TEXT_SKIP_DRAW, NULL);
      x += 8;
   }
   CopyWindowToVram(widget->window, COPYWIN_GFX);
   PutWindowTilemap(widget->window);
}
extern void SetTexteditValueCursorPos(struct LuTexteditValueWidget* widget, u8 pos) {
   if (pos > widget->max_length) {
      //
      // Setting the cursor position past the end of the displayed value 
      // is valid: do this when the cursor is "outside" of the text, e.g. 
      // because the user has inputted text up to the max length and can 
      // no longer add any more characters.
      //
      pos = widget->max_length;
   }
   if (widget->cursor_pos == pos) {
      return;
   }
   if (widget->cursor_pos < widget->max_length) {
      u8 sprite_id = widget->underscore_sprite_ids[widget->cursor_pos];
      gSprites[sprite_id].data[4] = FALSE;
   }
   widget->cursor_pos = pos;
   if (pos < widget->max_length) {
      u8 sprite_id = widget->underscore_sprite_ids[widget->cursor_pos];
      gSprites[sprite_id].data[4] = TRUE;
   }
}

extern void DestroyTexteditValueWidget(struct LuTexteditValueWidget* widget) {
   if (widget->owns_window) {
      RemoveWindow(widget->window);
      widget->window = WINDOW_NONE;
   }
   for(u8 i = 0; i < sizeof(widget->underscore_sprite_ids); ++i) {
      u8 sprite_id = widget->underscore_sprite_ids[i];
      if (sprite_id != SPRITE_NONE) {
         DestroySprite(&gSprites[sprite_id]);
         widget->underscore_sprite_ids[i] = SPRITE_NONE;
      }
   }
}

// --- Underscores ---

static void SpriteCB_Underscore(struct Sprite*);

static const union AnimCmd sAnim_Loop[] = {
   ANIMCMD_FRAME(0, 1),
   ANIMCMD_JUMP(0)
};
static const union AnimCmd *const sAnims_Loop[] = {
   sAnim_Loop
};
static const struct OamData sOam_8x8 = {
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x8),
    .x = 0,
    .size = SPRITE_SIZE(8x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
};
static const struct SpriteTemplate sSpriteTemplate_Underscore = {
    .tileTag     = 0,
    .paletteTag  = 0,
    .oam         = &sOam_8x8,
    .anims       = sAnims_Loop,
    .images      = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback    = SpriteCB_Underscore
};

static void LoadSpriteAssets(u16 tile_tag, u16 palette_tag) {
   const struct SpriteSheet spritesheets[] = {
      { gNamingScreenUnderscore_Gfx, 0x20, tile_tag },
      { 0 }
   };
   const struct SpritePalette palette = { gNamingScreenMenu_Pal[3], palette_tag };
   
   LoadSpriteSheets(spritesheets);
   LoadSpritePalette(&palette);
}

#define sIndex  data[0]
#define sYPosId data[1]
#define sDelay  data[2]
#define sActive data[3]

static void SpriteCB_Underscore(struct Sprite* sprite) {
   const s16 y[] = {2, 3, 2, 1};

   if (sprite->sActive == 0) {
      sprite->y2      = 0;
      sprite->sYPosId = 0;
      sprite->sDelay  = 0;
   } else {
      sprite->y2 = y[sprite->sYPosId];
      sprite->sDelay++;
      if (sprite->sDelay > 8) {
         sprite->sYPosId = MOD(sprite->sYPosId + 1, ARRAY_COUNT(y));
         sprite->sDelay  = 0;
      }
   }
}

#undef sIndex
#undef sYPosId
#undef sDelay
#undef sActive