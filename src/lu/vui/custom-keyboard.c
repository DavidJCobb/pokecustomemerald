#include "lu/vui/custom-keyboard.h"
#include "lu/vui/vui-frame.h"
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
#include "lu/c-attr.define.h"

static const u8 sCharsetCharactersUpper[] = __(
   "ABCDEF ."
   "GHIJKL ,"
   "MNOPRQS "
   "TUVWXYZ "
);
static const u8 sCharsetCharactersLower[] = __(
   "abcdef ."
   "ghijkl ,"
   "mnopqrs "
   "tuvwxyz "
);
static const u8 sCharsetCharactersSymbol[] = __(
   "01234 "
   "56789 "
   "!?♂♀/-"
   "…“”‘' "
);
/*extern*/ const struct VUICustomKeyboardCharset gVUICustomKeyboardDefaultCharsets[3] = {
   {
      .characters = sCharsetCharactersUpper,
      .rows = 4,
      .cols = 8,
      .col_gaps = {
         .count     = 2,
         .positions = { 2, 6 }
      }
   },
   {
      .characters = sCharsetCharactersLower,
      .rows = 4,
      .cols = 8,
      .col_gaps = {
         .count     = 2,
         .positions = { 2, 6 }
      }
   },
   {
      .characters = sCharsetCharactersSymbol,
      .rows = 4,
      .cols = 6,
      .col_gaps = {
         .count = 0
      }
   },
};

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
static u8 VFunc_OnSubgridFocusMoved(VUIWidget*);
//
static const struct VTable_VUIWidget sVTable = {
   &gVTable_VUIWidget,
   VFunc_DestroyImpl,
   VFunc_OnFrame,
   VFunc_OnFocusChange,
   VFunc_OnSubgridFocusMoved,
};

NON_NULL_PARAMS(1) static void AppendGlyph(VUICustomKeyboard*);
NON_NULL_PARAMS(1) static u8 GetCharacter(VUICustomKeyboard*, u8 x, u8 y);
NON_NULL_PARAMS(1) static u8 GetKeyColPx(VUICustomKeyboard*, u8 n);
NON_NULL_PARAMS(1) static u8 GetKeyRowPx(VUICustomKeyboard*, u8 n);
NON_NULL_PARAMS(1) static void RepaintKeys(VUICustomKeyboard*);
NON_NULL_PARAMS(1) static void UpdateCursorSprite(VUICustomKeyboard*);

static const struct SpritePalette sSpritePalettes[];
static const struct SpriteSheet sSpriteSheets[];
static const struct SpriteTemplate sSpriteTemplate_Cursor;
static void SpriteCB_Cursor(struct Sprite*);

// --------------------------------------------------------------------------------

extern void VUICustomKeyboard_Construct(VUICustomKeyboard* this) {
   VUIWidget_Construct(&this->base);
   this->base.functions   = &sVTable;
   this->base.has_subgrid = TRUE;
   this->base.context_controls_subgrid_focus = TRUE;
   
   this->charsets_count = 0;
   this->charset = 0;
   this->value.data = NULL;
   this->value.size = 0;
   this->cursor_sprite_id = SPRITE_NONE;
   this->window_id = WINDOW_NONE;
   
   this->callbacks.on_text_changed      = NULL;
   this->callbacks.on_text_at_maxlength = NULL;
}
extern void VUICustomKeyboard_Initialize(
   VUICustomKeyboard* this,
   const VUICustomKeyboard_InitParams* params
) {
   this->base.focusable = TRUE;
   VUIWidget_SetGridMetrics(this, params->grid.pos.x, params->grid.pos.y, params->grid.size.w, params->grid.size.h);
   
   this->value          = params->buffer;
   this->callbacks      = params->callbacks;
   this->charsets       = params->charsets;
   this->charsets_count = params->charsets_count;
   AGB_WARNING(!VUIStringRef_IsNull(&this->value));
   AGB_WARNING(this->charsets != NULL);
   AGB_WARNING(this->charsets_count > 0);
   #ifndef NDEBUG
      for(u8 i = 0; i < this->charsets_count; ++i) {
         AGB_WARNING(this->charsets[i].characters != NULL);
         AGB_WARNING(this->charsets[i].col_gaps.count < sizeof(this->charsets[i].col_gaps.positions));
      }
   #endif
   
   this->colors = params->colors;
   this->rendering.bg_layer = params->bg_layer;
   this->rendering.palette  = params->palette;
   
   {
      LoadSpriteSheets(sSpriteSheets);
      LoadSpritePalettes(sSpritePalettes);
      //
      u8 px_x = (params->tile_x + 1) * TILE_WIDTH  + 3;
      u8 px_y = (params->tile_y + 1) * TILE_HEIGHT + 9;
      //
      u8 sprite_id = CreateSprite(&sSpriteTemplate_Cursor, px_x, px_y, 1);
      if (sprite_id != SPRITE_NONE) {
         this->cursor_sprite_id = sprite_id;
         
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
      this->window_id = window_id;
      
      PutWindowTilemap(window_id);
   }
   if (params->frame) {
      VUIFrame_Draw(
         params->frame,
         params->tile_x,
         params->tile_y,
         VUIKEYBOARD_INNER_W_TILES + 2,
         VUIKEYBOARD_INNER_H_TILES + 2
      );
   }
   
   VUICustomKeyboard_SetCharset(this, 0);
}
extern void VUICustomKeyboard_Backspace(VUICustomKeyboard* this) {
   AGB_WARNING(!VUIStringRef_IsNull(&this->value));
   if (VUIStringRef_IsNull(&this->value))
      return;
   
   u16 len = StringLength(this->value.data);
   AGB_WARNING(len <= this->value.size);
   if (len == 0) {
      DebugPrintf("[VUIKeyboard] Can't backspace; string already empty.");
      return;
   }
   this->value.data[len - 1] = EOS;
   if (this->callbacks.on_text_changed)
      (this->callbacks.on_text_changed)(this->value.data);
}
extern void VUICustomKeyboard_NextCharset(VUICustomKeyboard* this) {
   VUICustomKeyboard_SetCharset(this, this->charset + 1);
}
extern void VUICustomKeyboard_SetCharset(VUICustomKeyboard* this, u8 id) {
   if (this->charsets_count == 0)
      return;
   id %= this->charsets_count;
   this->charset = id;
   {
      VUISize new_size;
      new_size.w = this->charsets[id].cols;
      new_size.h = this->charsets[id].rows;
      if (!new_size.w)
         new_size.w = COL_COUNT;
      if (!new_size.h)
         new_size.h = ROW_COUNT;
      this->base.subgrid_size = new_size;
      VUI_ConstrainPos(&this->base.subgrid_focus, &new_size);
   }
   UpdateCursorSprite(this);
   RepaintKeys(this);
}

// --------------------------------------------------------------------------------

static u8 VFunc_DestroyImpl(VUIWidget* widget) {
   VUICustomKeyboard* this = (VUICustomKeyboard*)widget;
   if (this->cursor_sprite_id != SPRITE_NONE) {
      DestroySprite(&gSprites[this->cursor_sprite_id]);
      this->cursor_sprite_id = SPRITE_NONE;
   }
   if (this->window_id != WINDOW_NONE) {
      RemoveWindow(this->window_id);
      this->window_id = WINDOW_NONE;
   }
}

static u8 VFunc_OnFrame(VUIWidget* widget) {
   VUICustomKeyboard* this = (VUICustomKeyboard*)widget;
   if (JOY_NEW(A_BUTTON)) {
      AppendGlyph(this);
      return 0;
   }
   return 0;
}

static void VFunc_OnFocusChange(VUIWidget* widget, bool8 gained, VUIWidget* other) {
   VUICustomKeyboard* this = (VUICustomKeyboard*)widget;
   if (this->cursor_sprite_id != SPRITE_NONE) {
      struct Sprite* sprite = &gSprites[this->cursor_sprite_id];
      sprite->invisible = !gained;
      UpdateCursorSprite(this);
   }
}
static u8 VFunc_OnSubgridFocusMoved(VUIWidget* widget) {
   VUICustomKeyboard* this = (VUICustomKeyboard*)widget;
   UpdateCursorSprite(this);
}

static void AppendGlyph(VUICustomKeyboard* this) {
   AGB_WARNING(!VUIStringRef_IsNull(&this->value));
   if (VUIStringRef_IsNull(&this->value))
      return;
   if (this->value.data[this->value.size - 1] != EOS) {
      if (this->callbacks.on_text_at_maxlength)
         (this->callbacks.on_text_at_maxlength)();
      return;
   }
   
   u8 character = GetCharacter(this, this->base.subgrid_focus.x, this->base.subgrid_focus.y);
   u8 i = 0;
   for(; i < this->value.size; ++i)
      if (this->value.data[i] == EOS)
         break;
   AGB_WARNING(i <= this->value.size);
   this->value.data[i]     = character;
   this->value.data[i + 1] = EOS;
   if (this->callbacks.on_text_changed)
      (this->callbacks.on_text_changed)(this->value.data);
   if (i == this->value.size - 1)
      if (this->callbacks.on_text_at_maxlength)
         (this->callbacks.on_text_at_maxlength)();
}
static u8 GetCharacter(VUICustomKeyboard* this, u8 x, u8 y) {
   AGB_ASSERT(this->charsets != NULL);
   AGB_ASSERT(this->charset < this->charsets_count);
   const struct VUICustomKeyboardCharset* charset = &this->charsets[this->charset];
   
   const u8* characters = charset->characters;
   u8 rows = charset->rows;
   u8 cols = charset->cols;
   AGB_ASSERT(rows != 0);
   AGB_ASSERT(cols != 0);
   AGB_ASSERT(characters != NULL);
   AGB_ASSERT(x < cols);
   AGB_ASSERT(y < rows);
   
   return characters[y * cols + x];
}
static u8 GetKeyColPx(VUICustomKeyboard* this, u8 n) {
   const u16 PRECISION   = 10;
   
   const u16 WIDGET_PX = VUIKEYBOARD_INNER_W_TILES * TILE_WIDTH;
   const u16 EXTRA_PX  = VUIKEYBOARD_PX_EXTRA_SPACING_FLAT + (WIDGET_PX - VUIKEYBOARD_PX_INNER_W);
   
   const struct VUICustomKeyboardCharset* charset = &this->charsets[this->charset];
   u8 cols      = charset->cols;
   u8 gaps_in   = charset->col_gaps.count;
   u8 gaps_past = 0;
   for(u8 i = 0; i < gaps_in; ++i) {
      u8 gap = charset->col_gaps.positions[i];
      if (n > gap)
         ++gaps_past;
   }
   
   // spx = scaled px, for division
   const u8  px_per_gap    = gaps_in ? EXTRA_PX / gaps_in : 0;
   const u16 spx_available = (WIDGET_PX - (gaps_in ? EXTRA_PX : 0)) * PRECISION;
   const u16 spx_per_col   = spx_available / cols;
   
   u8 wpx = spx_per_col * n / PRECISION + (gaps_past * px_per_gap);
   
   return wpx;
}
static u8 GetKeyRowPx(VUICustomKeyboard* this, u8 n) {
   return n * VUIKEYBOARD_PX_ROW_H;
}
static void RepaintKeys(VUICustomKeyboard* this) {
   FillWindowPixelBuffer(this->window_id, PIXEL_FILL(this->colors.back));
   
   const struct VUICustomKeyboardCharset* charset = &this->charsets[this->charset];
   
   const u8* characters = charset->characters;
   const u8  rows       = charset->rows;
   const u8  cols       = charset->cols;
   AGB_ASSERT(rows != 0);
   AGB_ASSERT(cols != 0);
   AGB_ASSERT(characters != NULL);
   
   u8 buffer[2];
   buffer[1] = EOS;
   
   for(u8 x = 0; x < cols; ++x) {
      u8 px_x = GetKeyColPx(this, x);
      for(u8 y = 0; y < rows; ++y) {
         u8 px_y = GetKeyRowPx(this, y);
         
         buffer[0] = characters[y * cols + x];
         
         u8 width  = GetStringWidth(FONT_NORMAL, buffer, 0);
         s8 offset = (VUIKEYBOARD_PX_COL_W - width) / 2;
         
         AddTextPrinterParameterized3(
            this->window_id,
            FONT_NORMAL,
            px_x + offset,
            px_y + 1,
            this->colors.list,
            TEXT_SKIP_DRAW,
            buffer
         );
      }
   }
   
   CopyWindowToVram(this->window_id, COPYWIN_FULL);
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

static void UpdateCursorSprite(VUICustomKeyboard* this) {
   if (this->cursor_sprite_id == SPRITE_NONE)
      return;
   struct Sprite* sprite = &gSprites[this->cursor_sprite_id];
   sprite->x2 = GetKeyColPx(this, this->base.subgrid_focus.x);
   sprite->y2 = GetKeyRowPx(this, this->base.subgrid_focus.y);
   
   sprite->sPrevCoords = sprite->sCoords;
   sprite->sCoords     = ((u16)this->base.subgrid_focus.y << 8) | this->base.subgrid_focus.x;
}