#include "lu/vui/keyboard-value.h"
#include "gba/isagbprint.h"
#include "bg.h"
#include "global.h" // ARRAY_COUNT
#include "graphics.h"
#include "menu.h"
#include "sprite.h"
#include "text.h"
#include "window.h"
#include "constants/characters.h"

static u8 VFunc_DestroyImpl(VUIWidget*);
static u8 VFunc_OnFrame(VUIWidget*);
static void VFunc_OnFocusChange(VUIWidget*, bool8, VUIWidget*);
//
static const struct VTable_VUIWidget sVTable = {
   &gVTable_VUIWidget,
   VFunc_DestroyImpl,
   NULL,
   NULL,
};

static const struct SpritePalette sSpritePalettes[];
static const struct SpriteSheet sSpriteSheets[];
static const struct SpriteTemplate sSpriteTemplate_Underscore;
static void SpriteCB_Underscore(struct Sprite*);

extern void VUIKeyboardValue_Construct(VUIKeyboardValue* this, const VUIKeyboardValue_InitParams* params) {
   VUIWidget_Construct(&this->base);
   this->base.functions = &sVTable;
   
   for(u8 i = 0; i < 10; ++i) {
      this->underscore_sprite_ids[i] = SPRITE_NONE;
   }
   
   this->colors = params->colors;
   
   //
   // TODO: Draw border
   //
   {
      const struct WindowTemplate tmpl = {
         .bg          = params->bg_layer,
         .tilemapLeft = params->tile_x + 1,
         .tilemapTop  = params->tile_y + 1,
         .width       = params->max_length,
         .height      = 2,
         .paletteNum  = params->palette,
         .baseBlock   = params->first_tile_id
      };
      u8 window_id = AddWindow(&tmpl);
      AGB_ASSERT(window_id != WINDOW_NONE);
      this->rendering.window_id = window_id;
      
      PutWindowTilemap(window_id);
   }
   
   LoadSpriteSheets(sSpriteSheets);
   LoadSpritePalettes(sSpritePalettes);
   //
   u8 x = (params->tile_x + 1) * TILE_WIDTH  + 2;
   u8 y = (params->tile_y + 1) * TILE_HEIGHT + 14;
   for(u8 i = 0; i < params->max_length; ++i) {
      u8 sprite_id = CreateSprite(&sSpriteTemplate_Underscore, x + (i * 8), y, 0);
      this->underscore_sprite_ids[i] = sprite_id;
      if (sprite_id != SPRITE_NONE) {
         struct Sprite* sprite = &gSprites[sprite_id];
         sprite->oam.priority = 0;
         sprite->data[0]      = 0;
         sprite->invisible    = TRUE;
      }
   }
   
   VUIKeyboardValue_ShowValue(this, NULL);
   
   DebugPrintf("[VUIKeyboardValue] Constructed on BG %u with window ID %u, from tile ID %u.", params->bg_layer, this->rendering.window_id, params->first_tile_id);
}

extern void VUIKeyboardValue_SetUnderscoreVisibility(VUIKeyboardValue* this, bool8 v) {
   for(u8 i = 0; i < sizeof(this->underscore_sprite_ids); ++i) {
      u8 sprite_id = this->underscore_sprite_ids[i];
      if (sprite_id == SPRITE_NONE)
         break;
      struct Sprite* sprite = &gSprites[sprite_id];
      sprite->invisible = !v;
   }
}

extern void VUIKeyboardValue_ShowValue(VUIKeyboardValue* this, const u8* string) {
   FillWindowPixelBuffer(this->rendering.window_id, PIXEL_FILL(this->colors.back));
   
   u8 buffer[2];
   buffer[1] = EOS;
   
   u8 length = 0;
   {
      u8 x = 0;
      u8 y = 0;
      for(const u8* c_ptr = string; c_ptr; ++c_ptr) {
         u8 c = *c_ptr;
         if (c == EOS)
            break;
         ++length;
         buffer[0] = c;
         AddTextPrinterParameterized3(
            this->rendering.window_id,
            FONT_NORMAL,
            x,
            y + 1,
            this->colors.list,
            TEXT_SKIP_DRAW,
            buffer
         );
         x += TILE_WIDTH;
      }
   }
   
   PutWindowTilemap(this->rendering.window_id);
   CopyWindowToVram(this->rendering.window_id, COPYWIN_FULL);
   
   for(u8 i = 0; i < sizeof(this->underscore_sprite_ids); ++i) {
      u8 sprite_id = this->underscore_sprite_ids[i];
      if (sprite_id == SPRITE_NONE)
         break;
      struct Sprite* sprite = &gSprites[sprite_id];
      sprite->data[0] = (i == length);
   }
}

// --------------------------------------------------------------------------------

static u8 VFunc_DestroyImpl(VUIWidget* widget) {
   VUIKeyboardValue* this = (VUIKeyboardValue*)widget;
   for(u8 i = 0; i < 10; ++i) {
      u8 sprite_id = this->underscore_sprite_ids[i];
      if (sprite_id != SPRITE_NONE) {
         DebugPrintf("[VUIKeyboardValue][Destroy-Impl] Destroying underscore %u (sprite ID %u)...", i, sprite_id);
         DestroySprite(&gSprites[sprite_id]);
         this->underscore_sprite_ids[i] = SPRITE_NONE;
      }
   }
   if (this->rendering.window_id != WINDOW_NONE) {
      DebugPrintf("[VUIKeyboardValue][Destroy-Impl] Destroying window ID %u...", this->rendering.window_id);
      RemoveWindow(this->rendering.window_id);
      this->rendering.window_id = WINDOW_NONE;
   }
}

// --------------------

static const struct SpritePalette sSpritePalettes[] = {
   {gNamingScreenMenu_Pal[3], VUIKEYBOARDVALUE_SPRITE_PALETTE_TAG},
   {}
};
static const struct SpriteSheet sSpriteSheets[] = {
   {gNamingScreenUnderscore_Gfx, TILE_SIZE_4BPP, VUIKEYBOARDVALUE_SPRITE_GRAPHIC_TAG},
   {}
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
static const union AnimCmd sAnim_Loop[] = {
   ANIMCMD_FRAME(0, 1),
   ANIMCMD_JUMP(0)
};
static const union AnimCmd *const sAnims_Loop[] = {
   sAnim_Loop
};
static const struct SpriteTemplate sSpriteTemplate_Underscore = {
   .tileTag     = VUIKEYBOARDVALUE_SPRITE_GRAPHIC_TAG,
   .paletteTag  = VUIKEYBOARDVALUE_SPRITE_PALETTE_TAG,
   .oam         = &sOam_8x8,
   .anims       = sAnims_Loop,
   .images      = NULL,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback    = SpriteCB_Underscore
};

#define sActive data[0]
#define sYPosId data[1]
#define sDelay  data[2]

static void SpriteCB_Underscore(struct Sprite* sprite) {
   const s16 y[] = {2, 3, 2, 1};
   if (sprite->sActive) {
      sprite->y2 = y[sprite->sYPosId];
      sprite->sDelay++;
      if (sprite->sDelay > 8) {
         sprite->sYPosId = MOD(sprite->sYPosId + 1, ARRAY_COUNT(y));
         sprite->sDelay  = 0;
      }
   } else {
      sprite->y2      = 0;
      sprite->sYPosId = 0;
      sprite->sDelay  = 0;
   }
}

#undef sActive
#undef sYPosId
#undef sDelay