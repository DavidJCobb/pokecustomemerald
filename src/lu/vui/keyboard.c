#include "lu/vui/keyboard.h"
#include "gba/isagbprint.h"
#include "global.h" // __()
#include "field_effect.h" // MultiplyInvertedPaletteRGBComponents
#include "graphics.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"
#include "window.h"
#include "constants/characters.h"

enum {
   COL_COUNT = 8,
   ROW_COUNT = VUIKEYBOARD_MAX_GRID_ROWS,
   
   COL_GROUP_LIST_LENGTH = 3,
   
   GRID_CELL_TILE_WIDTH  = 1,
   GRID_CELL_TILE_HEIGHT = 2,
   
   GRID_WIDTH  = VUIKEYBOARD_MAX_GRID_COLS,
   GRID_HEIGHT = VUIKEYBOARD_MAX_GRID_ROWS,
};

static u8 VFunc_DestroyImpl(VUIWidget*);
static u8 VFunc_OnFrame(VUIWidget*);
static void VFunc_OnFocusChange(VUIWidget*, bool8, VUIWidget*);
//
static const struct VTable_VUIWidget sVTable = {
   &gVTable_VUIWidget,
   VFunc_DestroyImpl,
   VFunc_OnFrame,
   VFunc_OnFocusChange,
};

static const u8 sCharsetChars[VUIKEYBOARD_CHARSET_COUNT][ROW_COUNT][COL_COUNT] = {
   [VUIKEYBOARD_CHARSET_UPPER] = {
      __("ABCDEF ."),
      __("GHIJKL ,"),
      __("MNOPRQS "),
      __("TUVWXYZ "),
   },
   [VUIKEYBOARD_CHARSET_LOWER] = {
      __("abcdef ."),
      __("ghijkl ,"),
      __("mnopqrs "),
      __("tuvwxyz "),
   },
   [VUIKEYBOARD_CHARSET_SYMBOLS] = {
      __("01234   "),
      __("56789   "),
      __("!?♂♀/-  "),
      __("…“”‘'   "),
   },
};
static const struct CharsetInfo {
   const u8* characters;
   u8 cols;
} sCharsets[] = {
   {
      .cols = 8
   },
   {
      .cols = 8
   },
   {
      .cols = 6
   },
};

static u8 GetColCount(const VUIKeyboard*);
static void MoveCursor(VUIKeyboard*, s8 x, s8 y);
static void AppendGlyph(VUIKeyboard*);
static u8 GetKeyColPx(VUIKeyboard*, u8 n);
static u8 GetKeyRowPx(VUIKeyboard*, u8 n);
static void RepaintKeys(VUIKeyboard*);
static void UpdateCursorSprite(VUIKeyboard*);

static const struct SpritePalette sSpritePalettes[];
static const struct SpriteSheet sSpriteSheets[];
static const struct SpriteTemplate sSpriteTemplate_Cursor;
static void SpriteCB_Cursor(struct Sprite*);

// --------------------------------------------------------------------------------

extern void VUIKeyboard_Construct(
   VUIKeyboard* widget,
   const VUIKeyboard_InitParams* params
) {
   VUIWidget_Construct(&widget->base);
   widget->base.functions   = &sVTable;
   widget->base.focusable   = TRUE;
   widget->base.has_subgrid = TRUE;
   
   widget->callbacks.on_text_changed      = NULL;
   widget->callbacks.on_text_at_maxlength = NULL;
   
   widget->rendering.window_id = WINDOW_NONE;
   
   widget->value.buffer     = NULL;
   widget->value.max_length = 0;
   widget->charset = 0;
   widget->cursor_sprite_id = SPRITE_NONE;
   
   widget->colors = params->colors;
   widget->rendering.bg_layer = params->bg_layer;
   widget->rendering.palette  = params->palette;
   
   {
      LoadSpriteSheets(sSpriteSheets);
      LoadSpritePalettes(sSpritePalettes);
      //
      u8 px_x = (params->tile_x + 1) * TILE_WIDTH  + 2;
      u8 px_y = (params->tile_y + 1) * TILE_HEIGHT + 9;
      //
      u8 sprite_id = CreateSprite(&sSpriteTemplate_Cursor, px_x, px_y, 1);
      if (sprite_id != SPRITE_NONE) {
         widget->cursor_sprite_id = sprite_id;
         
         struct Sprite* sprite = &gSprites[sprite_id];
         sprite->oam.priority = 1;
         sprite->oam.objMode  = ST_OAM_OBJ_BLEND;
         sprite->data[2]      = 1; // sFlashing
         sprite->data[4]      = 2; // sColorIncr
      }
   }
   {
      const struct WindowTemplate tmpl = {
         .bg          = params->bg_layer,
         .tilemapLeft = params->tile_x + 1,
         .tilemapTop  = params->tile_y + 1,
         .width       = VUIKEYBOARD_INNER_W_TILES,
         .height      = VUIKEYBOARD_INNER_H_TILES,
         .paletteNum  = params->palette,
         .baseBlock   = params->first_tile_id
      };
      u8 window_id = AddWindow(&tmpl);
      AGB_ASSERT(window_id != WINDOW_NONE);
      widget->rendering.window_id = window_id;
      
      PutWindowTilemap(window_id);
   }
   
   VUIKeyboard_SetCharset(widget, 0);
}
extern void VUIKeyboard_Backspace(VUIKeyboard* this) {
   AGB_WARNING(!!this->value.buffer && this->value.max_length > 0);
   if (!this->value.buffer || !this->value.max_length)
      return;
   
   u16 len = StringLength(this->value.buffer);
   AGB_WARNING(len <= this->value.max_length);
   if (len == 0) {
      DebugPrintf("[VUIKeyboard] Can't backspace; string already empty.");
      return;
   }
   this->value.buffer[len - 1] = EOS;
   if (this->callbacks.on_text_changed)
      (this->callbacks.on_text_changed)(this->value.buffer);
}
extern void VUIKeyboard_NextCharset(VUIKeyboard* this) {
   VUIKeyboard_SetCharset(this, this->charset + 1);
}
extern void VUIKeyboard_SetCharset(VUIKeyboard* this, enum VUIKeyboardCharsetID id) {
   id %= VUIKEYBOARD_CHARSET_COUNT;
   this->charset = id;
   {
      VUISize new_size;
      new_size.w = sCharsets[id].cols;
      new_size.h = ROW_COUNT;
      if (!new_size.w)
         new_size.w = COL_COUNT;
      this->base.subgrid_size = new_size;
      VUI_ConstrainPos(&this->base.subgrid_focus, &new_size);
   }
   UpdateCursorSprite(this);
   RepaintKeys(this);
}

// --------------------------------------------------------------------------------

static u8 VFunc_DestroyImpl(VUIWidget* widget) {
   VUIKeyboard* this = (VUIKeyboard*)widget;
   if (this->cursor_sprite_id != SPRITE_NONE) {
      DestroySprite(&gSprites[this->cursor_sprite_id]);
      this->cursor_sprite_id = SPRITE_NONE;
   }
   if (this->rendering.window_id != WINDOW_NONE) {
      RemoveWindow(this->rendering.window_id);
      this->rendering.window_id = WINDOW_NONE;
   }
}

static u8 VFunc_OnFrame(VUIWidget* widget) {
   VUIKeyboard* this = (VUIKeyboard*)widget;
   if (JOY_NEW(A_BUTTON)) {
      AppendGlyph(this);
      return VWIDGET_FRAMEHANDLER_CONSUMED_DPAD;
   }
   
   s8 move_x = 0;
   s8 move_y = 0;
   if (JOY_NEW(DPAD_LEFT)) {
      move_x = -1;
   } else if (JOY_NEW(DPAD_RIGHT)) {
      move_x = 1;
   } else if (JOY_NEW(DPAD_UP)) {
      move_y = -1;
   } else if (JOY_NEW(DPAD_DOWN)) {
      move_y = 1;
   }
   
   if (
      (move_x < 0 && this->base.subgrid_focus.x == 0)
   || (move_x > 0 && this->base.subgrid_focus.x + 1 == GetColCount(this))
   ) {
      return 0;
   }
   if (move_x || move_y)
      MoveCursor(this, move_x, move_y);
   return VWIDGET_FRAMEHANDLER_CONSUMED_DPAD;
}

static void VFunc_OnFocusChange(VUIWidget* widget, bool8 gained, VUIWidget* other) {
   VUIKeyboard* this = (VUIKeyboard*)widget;
   if (this->cursor_sprite_id != SPRITE_NONE) {
      struct Sprite* sprite = &gSprites[this->cursor_sprite_id];
      sprite->invisible = !gained;
      UpdateCursorSprite(this);
   }
}

static u8 GetColCount(const VUIKeyboard* this) {
   u8 cols = sCharsets[this->charset].cols;
   if (cols == 0)
      cols = COL_COUNT;
   return cols;
}
static void MoveCursor(VUIKeyboard* this, s8 x, s8 y) {
   if (x) {
      u8 col_count = GetColCount(this);
      this->base.subgrid_focus.x += x;
      if (this->base.subgrid_focus.x >= col_count)
         this->base.subgrid_focus.x %= col_count;
   }
   if (y) {
      this->base.subgrid_focus.y += y;
      if (this->base.subgrid_focus.y >= ROW_COUNT)
         this->base.subgrid_focus.y %= ROW_COUNT;
   }
   //
   // Update sprite.
   //
   UpdateCursorSprite(this);
}
static void AppendGlyph(VUIKeyboard* this) {
   AGB_WARNING(!!this->value.buffer && this->value.max_length > 0);
   if (!this->value.buffer || !this->value.max_length)
      return;
   if (this->value.buffer[this->value.max_length - 1] != EOS) {
      if (this->callbacks.on_text_at_maxlength)
         (this->callbacks.on_text_at_maxlength)();
      return;
   }
   
   u8 character = sCharsetChars[this->charset][this->base.subgrid_focus.y][this->base.subgrid_focus.x];
   u8 i = 0;
   for(; i < this->value.max_length; ++i)
      if (this->value.buffer[i] == EOS)
         break;
   AGB_WARNING(i <= this->value.max_length);
   this->value.buffer[i]     = character;
   this->value.buffer[i + 1] = EOS;
   if (this->callbacks.on_text_changed)
      (this->callbacks.on_text_changed)(this->value.buffer);
   if (i == this->value.max_length - 1)
      if (this->callbacks.on_text_at_maxlength)
         (this->callbacks.on_text_at_maxlength)();
}
static u8 GetKeyColPx(VUIKeyboard* this, u8 n) {
   const u16 WIDGET_PX   = VUIKEYBOARD_PX_INNER_W;
   const u16 COLUMN_PX   = VUIKEYBOARD_PX_COL_W;
   const u8  GAP_SIZE_PX = VUIKEYBOARD_PX_EXTRA_SPACING_FLAT / 2;
   
   u8 real_cols = GetColCount(this);
   u8 draw_cols = real_cols;
   u8 cursor_x  = n;
   u8 gaps_in   = 0;
   u8 gaps_past = 0;
   switch (this->charset) {
      case VUIKEYBOARD_CHARSET_UPPER:
      case VUIKEYBOARD_CHARSET_LOWER:
         //
         // We divide keyboard rows into three groups:
         //
         //    xxx xxxx x
         //
         gaps_in = 2;
         if (cursor_x > 6) {
            gaps_past = 2;
         } else if (cursor_x > 2) {
            gaps_past = 1;
         }
         break;
   }
   
   u8 wpx = (WIDGET_PX - (gaps_in * GAP_SIZE_PX)) * 10 / draw_cols * cursor_x / 10 + (gaps_past * GAP_SIZE_PX);
   
   return wpx;
}
static u8 GetKeyRowPx(VUIKeyboard* this, u8 n) {
   return n * VUIKEYBOARD_PX_ROW_H;
}
static void RepaintKeys(VUIKeyboard* this) {
   FillWindowPixelBuffer(this->rendering.window_id, PIXEL_FILL(this->colors.back));
   
   const struct CharsetInfo* info = &sCharsets[this->charset];
   
   u8 cols = info->cols;
   if (!cols)
      cols = COL_COUNT;
   u8 char_count = cols * ROW_COUNT;
   
   u8 buffer[2];
   buffer[1] = EOS;
   
   for(u8 x = 0; x < cols; ++x) {
      u8 px_x = GetKeyColPx(this, x);
      for(u8 y = 0; y < ROW_COUNT; ++y) {
         u8 px_y = GetKeyRowPx(this, y);
         
         buffer[0] = sCharsetChars[this->charset][y][x];
         
         u8 width  = GetStringWidth(FONT_NORMAL, buffer, 0);
         u8 offset = (VUIKEYBOARD_PX_COL_W - width) / 2;
         
         AddTextPrinterParameterized3(
            this->rendering.window_id,
            FONT_NORMAL,
            px_x + offset,
            px_y + 1,
            this->colors.list,
            TEXT_SKIP_DRAW,
            buffer
         );
      }
   }
   
   CopyWindowToVram(this->rendering.window_id, COPYWIN_FULL);
}




static const struct SpritePalette sSpritePalettes[] = {
   {gNamingScreenMenu_Pal[5], VUIKEYBOARD_SPRITE_PALETTE_TAG},
   {}
};
static const struct SpriteSheet sSpriteSheets[] = {
   {gNamingScreenCursor_Gfx, TILE_SIZE_4BPP*4, VUIKEYBOARD_SPRITE_GRAPHIC_TAG},
   {}
};
static const struct OamData sOam_16x16 = {
   .y          = 0,
   .affineMode = ST_OAM_AFFINE_OFF,
   .objMode    = ST_OAM_OBJ_NORMAL,
   .bpp        = ST_OAM_4BPP,
   .shape      = SPRITE_SHAPE(16x16),
   .x          = 0,
   .size       = SPRITE_SIZE(16x16),
   .tileNum    = 0,
   .priority   = 0,
   .paletteNum = 0,
};
static const union AnimCmd sAnim_Loop[] = {
   ANIMCMD_FRAME(0, 1),
   ANIMCMD_JUMP(0)
};
static const union AnimCmd sAnim_CursorSquish[] = {
   ANIMCMD_FRAME(4, 8),
   ANIMCMD_FRAME(8, 8),
   ANIMCMD_END
};
static const union AnimCmd *const sAnims_Cursor[] = {
   sAnim_Loop,
   sAnim_CursorSquish
};
static const struct SpriteTemplate sSpriteTemplate_Cursor = {
   .tileTag     = VUIKEYBOARD_SPRITE_GRAPHIC_TAG,
   .paletteTag  = VUIKEYBOARD_SPRITE_PALETTE_TAG,
   .oam         = &sOam_16x16,
   .anims       = sAnims_Cursor,
   .images      = NULL,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback    = SpriteCB_Cursor
};

#define sCoords     data[0]
#define sPrevCoords data[1]
#define sFlashing   data[2]
#define sColor      data[3]
#define sColorIncr  data[4]
#define sColorDelay data[5]

static void SpriteCB_Cursor(struct Sprite* sprite) {
   if (sprite->animEnded)
      StartSpriteAnim(sprite, 0);

   if (sprite->invisible
   || !sprite->sFlashing
   || sprite->sCoords != sprite->sPrevCoords
   ) {
      sprite->sColor      = 0;
      sprite->sColorIncr  = 2;
      sprite->sColorDelay = 2;
   }

   sprite->sColorDelay--;
   if (sprite->sColorDelay == 0) {
      sprite->sColor += sprite->sColorIncr;
      if (sprite->sColor == 16 || sprite->sColor == 0)
         sprite->sColorIncr = -sprite->sColorIncr;
      sprite->sColorDelay = 2;
   }

   if (sprite->sFlashing) {
      s8  gb    = sprite->sColor;
      s8  r     = sprite->sColor >> 1;
      u16 index = OBJ_PLTT_ID(IndexOfSpritePaletteTag(VUIKEYBOARD_SPRITE_PALETTE_TAG)) + 1;

      MultiplyInvertedPaletteRGBComponents(index, r, gb, gb);
   }
}

static void UpdateCursorSprite(VUIKeyboard* this) {
   if (this->cursor_sprite_id == SPRITE_NONE)
      return;
   struct Sprite* sprite = &gSprites[this->cursor_sprite_id];
   sprite->x2 = GetKeyColPx(this, this->base.subgrid_focus.x);
   sprite->y2 = GetKeyRowPx(this, this->base.subgrid_focus.y);
   
   sprite->sPrevCoords = sprite->sCoords;
   sprite->sCoords     = ((u16)this->base.subgrid_focus.y << 8) | this->base.subgrid_focus.x;
}