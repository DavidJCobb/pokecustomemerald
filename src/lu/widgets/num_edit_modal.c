#include "lu/widgets/num_edit_modal.h"
#include "constants/characters.h"
#include "gba/gba.h"
#include "gba/isagbprint.h"
#include "global.h"
#include "bg.h"
#include "graphics.h"
#include "field_effect.h" // MultiplyInvertedPaletteRGBComponents
#include "main.h"
#include "malloc.h"
#include "menu.h" // LoadMessageBoxAndBorderGfx
#include "palette.h"
#include "sprite.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "lu/ui_helpers.h"

static u8 CountDigitsIn(LuNumEditModalValue);

static u8 CreateCursorSprite(u16 tile_tag, u16 palette_tag);
static void UpdateCursorSprite(struct LuNumEditModal*);

static void RepaintValue(struct LuNumEditModal*);
static void LuNumEditModal_Task(u8);

extern void InitNumEditModal(
   struct LuNumEditModal* widget,
   const struct LuNumEditModalInitParams* params
) {
   AGB_ASSERT(params->min_value < params->max_value);
   
   LoadMessageBoxAndBorderGfx();
   
   widget->heap_free_on_destroy = FALSE;
   widget->cursor_sprite_id = SPRITE_NONE;
   widget->callback = params->callback;
   
   bool8 sign_can_vary = FALSE;
   if (params->min_value < 0 && params->max_value > 0)
      sign_can_vary = TRUE;
   
   u8 tile_x = params->window.x;
   u8 tile_y = params->window.y;
   u8 tile_w = 2;
   u8 tile_h = 4;
   {
      widget->min_value   = params->min_value;
      widget->max_value   = params->max_value;
      widget->cur_value   = params->cur_value;
      widget->digit_count = 0;
      
      u8 a = CountDigitsIn(params->min_value);
      u8 b = CountDigitsIn(params->max_value);
      if (params->min_value < 0)
         ++a;
      if (params->max_value < 0)
         ++b;
      
      widget->digit_count = (a < b) ? b : a;
   }
   tile_w += widget->digit_count;
   if (widget->min_value < 0)
      ++tile_w;
   
   if (tile_x + tile_w > DISPLAY_TILE_WIDTH)
      tile_x = DISPLAY_TILE_WIDTH - tile_w;
   if (tile_y + tile_h > DISPLAY_TILE_HEIGHT)
      tile_y = DISPLAY_TILE_HEIGHT - tile_h;
   
   widget->text_colors.back   = params->text_colors.back;
   widget->text_colors.text   = params->text_colors.text;
   widget->text_colors.shadow = params->text_colors.shadow;
   
   DebugPrintf("[LuNumEditModal][Init] Spawning paint window...");
   {  // Create window.
      const struct WindowTemplate tmpl = {
         .bg          = params->window.bg_layer,
         .tilemapLeft = tile_x + 1,
         .tilemapTop  = tile_y + 1,
         .width       = tile_w - 2,
         .height      = 2,
         .paletteNum  = params->window.palette_id,
         .baseBlock   = params->window.first_tile_id
      };
      u8 window_id = AddWindow(&tmpl);
      AGB_ASSERT(window_id != WINDOW_NONE);
      widget->window_id = window_id;
      
      PutWindowTilemap(window_id);
      FillWindowPixelBuffer(window_id, PIXEL_FILL(widget->text_colors.back));
      CopyWindowToVram(window_id, COPYWIN_FULL);
      
      LuUI_DrawWindowFrame(
         params->window.bg_layer,
         0x214,          // see LoadMessageBoxAndBorderGfx
         BG_PLTT_ID(15), // see LoadMessageBoxAndBorderGfx
         tile_x + 1,
         tile_y + 1,
         tile_w - 2,
         tile_h - 2
      );
      CopyBgTilemapBufferToVram(params->window.bg_layer);
   }
   DebugPrintf("[LuNumEditModal][Init] Paint window ready.");
   
   widget->cursor_sprite_id = CreateCursorSprite(
      params->sprite_tags.cursor.tile,
      params->sprite_tags.cursor.palette
   );
   DebugPrintf("[LuNumEditModal][Init] Cursor sprite ID is %u.", widget->cursor_sprite_id);
   {
      struct Sprite* sprite = &gSprites[widget->cursor_sprite_id];
      sprite->oam.priority = 1;
      sprite->oam.objMode = ST_OAM_OBJ_BLEND;
      sprite->x = 8 + (tile_x + 1) * TILE_WIDTH;
      sprite->y = 8 + (tile_y + 1) * TILE_HEIGHT;
      sprite->invisible = FALSE;
   }
   
   RepaintValue(widget);
   
   widget->task_id = TASK_NONE;
   if (params->use_task) {
      widget->task_id = CreateTask(LuNumEditModal_Task, 50);
      AGB_ASSERT(widget->task_id != TASK_NONE);
      SetWordTaskArg(widget->task_id, 0, (u32)widget);
   }
   widget->active = TRUE;
}

#define sX          data[0]
#define sY          data[1]
#define sPrevX      data[2]
#define sPrevY      data[3]
#define sColorState data[4]
#define sPaletteTag data[5]

struct CursorSpriteColorState {
   u8    color;     // 4 bits
   s8    increment; // 4 bits
   u8    delay;     // 2 bits
   bool8 flashing;  // 1 bit
};

#define UNPACK(v, offset, bitcount) ((v) >> (offset)) & ((1 << (bitcount)) - 1)
#define PACK(v, offset, bitcount) (((u16)(v) & ((1 << (bitcount)) - 1)) << (offset))
static void GetCursorSpriteColorState(struct Sprite* sprite, struct CursorSpriteColorState* dst) {
   const u16 packed = sprite->sColorState;
   dst->color     = UNPACK(packed,  0, 4);
   dst->increment = UNPACK(packed,  4, 4);
   dst->delay     = UNPACK(packed,  8, 3);
   dst->flashing  = UNPACK(packed, 11, 1);
}
static void SetCursorSpriteColorState(struct Sprite* sprite, struct CursorSpriteColorState* src) {
   u16 state = 0;
   state |= PACK(src->color,      0, 4);
   state |= PACK(src->increment,  4, 4);
   state |= PACK(src->delay,      8, 3);
   state |= PACK(src->flashing,  11, 1);
   sprite->sColorState = state;
}
#undef UNPACK
#undef PACK

static void SpriteCB_Cursor(struct Sprite* sprite) {
   if (sprite->animEnded)
      StartSpriteAnim(sprite, 0);

   struct CursorSpriteColorState state;
   GetCursorSpriteColorState(sprite, &state);

   if (
      sprite->invisible
   || !state.flashing
   || sprite->sX != sprite->sPrevX
   || sprite->sY != sprite->sPrevY
   ) {
      state.color      = 0;
      state.increment  = 2;
      state.delay      = 2;
   }

   state.delay--;
   if (state.delay == 0) {
      state.color += state.increment;
      if (state.color == 16 || state.color == 0)
         state.increment = -state.increment;
      state.delay = 2;
   }

   if (state.flashing) {
      s8  gb    = state.color;
      s8  r     = state.color >> 1;
      u16 index = OBJ_PLTT_ID(IndexOfSpritePaletteTag(sprite->sPaletteTag)) + 1;

      MultiplyInvertedPaletteRGBComponents(index, r, gb, gb);
   }
   
   SetCursorSpriteColorState(sprite, &state);
}

static const struct OamData sOam_16x16 = {
   .y = 0,
   .affineMode = ST_OAM_AFFINE_OFF,
   .objMode = ST_OAM_OBJ_NORMAL,
   .bpp = ST_OAM_4BPP,
   .shape = SPRITE_SHAPE(16x16),
   .x = 0,
   .size = SPRITE_SIZE(16x16),
   .tileNum = 0,
   .priority = 0,
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
static const struct SpriteFrameImage sImageTable_Cursor[] = {
   { gNamingScreenCursor_Gfx, 0x80 },
};

static u8 CreateCursorSprite(u16 tile_tag, u16 palette_tag) {
   const struct SpritePalette palette = { gNamingScreenMenu_Pal[5], palette_tag };
   DebugPrintf("[LuNumEditModal][Init] Loading cursor sprite palette...");
   LoadSpritePalette(&palette);
   
   /*//
   const struct SpriteSheet spritesheets[] = {
      { gNamingScreenCursor_Gfx, 0x80, tile_tag },
      { 0 }
   };
   DebugPrintf("[LuNumEditModal][Init] Loading cursor spritesheet...");
   LoadSpriteSheets(spritesheets);
   //*/
   
   const struct SpriteTemplate tmpl = {
      .tileTag     = TAG_NONE,
      .paletteTag  = palette_tag,
      .oam         = &sOam_16x16,
      .anims       = sAnims_Cursor,
      .images      = sImageTable_Cursor,
      .affineAnims = gDummySpriteAffineAnimTable,
      .callback    = SpriteCB_Cursor
   };
   
   DebugPrintf("[LuNumEditModal][Init] Spawning cursor sprite...");
   u8 sprite_id = CreateSprite(&tmpl, 8, 8, 1);
   
   struct Sprite* sprite = &gSprites[sprite_id];
   SpriteTakeOwnershipOfTemplate(sprite);
   {
      struct CursorSpriteColorState state = { 0 };
      state.increment = 2;
      SetCursorSpriteColorState(sprite, &state);
   }
   sprite->sPaletteTag = palette_tag;
   
   return sprite_id;
}
static void UpdateCursorSprite(struct LuNumEditModal* widget) {
   struct Sprite* sprite = &gSprites[widget->cursor_sprite_id];
   
   sprite->x2 = widget->cursor_pos * 8;
   
   sprite->sPrevX = sprite->sX;
   sprite->sPrevY = sprite->sY;
   sprite->sX     = sprite->x + sprite->x2;
   sprite->sY     = sprite->y + sprite->y2;
}

#undef sX
#undef sY
#undef sPrevX
#undef sPrevY
#undef sInvisible
#undef sFlashing
#undef sColor
#undef sColorIncr
#undef sColorDelay

static void ModCurrentDigit(struct LuNumEditModal*, s8 by);
static void MoveCursor(struct LuNumEditModal*, s8 by);

extern void HandleNumEditModalInput(struct LuNumEditModal* widget) {
   if (!widget->active)
      return;
   
   if (JOY_NEW(A_BUTTON)) {
      DebugPrintf("[LuNumEditModal] Pressed A!");
      LuNumEditModalCallback callback = widget->callback;
      LuNumEditModalValue    value    = widget->cur_value;
      
      DestroyNumEditModal(widget);
      if (callback) {
         callback(TRUE, value);
      }
      return;
   }
   if (JOY_NEW(B_BUTTON)) {
      DebugPrintf("[LuNumEditModal] Pressed B!");
      LuNumEditModalCallback callback = widget->callback;
      LuNumEditModalValue    value    = widget->cur_value;
      
      DestroyNumEditModal(widget);
      if (callback) {
         callback(FALSE, value);
      }
      return;
   }
   
   {  // Modify current digit.
      s8 by = 0;
      if (JOY_REPEAT(DPAD_UP)) {
         by = 1;
      } else if (JOY_REPEAT(DPAD_DOWN)) {
         by = -1;
      }
      if (by != 0) {
         DebugPrintf("[LuNumEditModal] Modifying digit by %d...", by);
         ModCurrentDigit(widget, by);
         return;
      }
   }
   
   {  // Move cursor.
      s8 by = 0;
      if (JOY_REPEAT(DPAD_LEFT))
         by = -1;
      else if (JOY_REPEAT(DPAD_RIGHT))
         by = 1;
      
      if (by != 0) {
         DebugPrintf("[LuNumEditModal] Moving cursor by %d...", by);
         MoveCursor(widget, by);
         return;
      }
   }
}

static void LuNumEditModal_Task(u8 task_id) {
   struct LuNumEditModal* widget = (struct LuNumEditModal*) GetWordTaskArg(task_id, 0);
   HandleNumEditModalInput(widget);
}

static const u32 powers_of_ten[] = {
   1,
   10,
   100,
   1000,
   10000,
   100000,
   1000000,
   10000000,
   100000000,
   1000000000,
};
static u8 CountDigitsIn(LuNumEditModalValue v) {
   if (v == 0)
      return 1;
   if (v < 0)
      v = -v;
   for(u8 i = 0; i < 10; ++i)
      if (v < powers_of_ten[i])
         return i;
   return 10;
}

static void ModCurrentDigit(struct LuNumEditModal* widget, s8 by) {
   u8 digit = widget->cursor_pos; // measured from most-significant to least-significant
   if (widget->min_value < 0 && widget->max_value > 0) {
      //
      // When the value's sign can vary, the first cursor position edits 
      // the sign, not the most-significant digit.
      //
      if (digit == 0) {
         widget->cur_value = -widget->cur_value;
         RepaintValue(widget);
         return;
      }
      --digit;
   }
   
   LuNumEditModalValue abs_value = widget->cur_value;
   if (abs_value < 0)
      abs_value = -abs_value;
   
   u8 power_index = widget->digit_count - digit - 1;
   
   LuNumEditModalValue less_significant = abs_value % powers_of_ten[power_index];
   LuNumEditModalValue more_significant;
   u8 digit_value;
   {
      LuNumEditModalValue big_part = abs_value / powers_of_ten[power_index];
      more_significant = big_part / 10 * 10;
      digit_value      = big_part - more_significant;
      more_significant *= powers_of_ten[power_index];
   }
   DebugPrintf("[LuNumEditModal][ModCurrentDigit] Digit %u (%u) is %d...", digit, powers_of_ten[power_index]*digit_value, digit_value);
   //
   // Example: extracting 3 from 12345:
   //
   //    digit       == 2
   //    digit_count == 5
   //    power_index == 2; powers_of_ten[power_index] == 100
   //
   //    big_part             == 123
   //    more_significant (a) == 120
   //    digit_value          ==   3
   //    more_significant (b) == 12000
   //
   
   if (by < 0) {
      if (digit_value == 0) {
         digit_value = 9;
      } else {
         --digit_value;
      }
   } else if (by > 0) {
      if (digit_value == 9) {
         digit_value = 0;
      } else {
         ++digit_value;
      }
   }
   
   abs_value = less_significant + more_significant + (digit_value * powers_of_ten[power_index]);
   if (widget->cur_value < 0)
      abs_value = -abs_value;
   widget->cur_value = abs_value;
   
   if (widget->cur_value < widget->min_value)
      widget->cur_value = widget->min_value;
   else if (widget->cur_value > widget->max_value)
      widget->cur_value = widget->max_value;
   
   RepaintValue(widget);
}
static void MoveCursor(struct LuNumEditModal* widget, s8 by) {
   if (by < 0) {
      if (widget->cursor_pos == 0)
         return;
      --widget->cursor_pos;
   } else if (by > 0) {
      u8 max = widget->digit_count;
      if (widget->min_value < 0 || widget->max_value > 0)
         ++max;
      if (widget->cursor_pos == max)
         return;
      ++widget->cursor_pos;
   }
   
   UpdateCursorSprite(widget);
}
static void RepaintValue(struct LuNumEditModal* widget) {
   FillWindowPixelBuffer(widget->window_id, PIXEL_FILL(widget->text_colors.back));
   
   u8 x_pos = 0;
   
   u8 buffer[2];
   buffer[1] = EOS;
   
   if (widget->min_value < 0) {
      if (widget->cur_value < 0) {
         buffer[1] = CHAR_HYPHEN;
         AddTextPrinterParameterized(widget->window_id, FONT_NORMAL, buffer, x_pos, 1, TEXT_SKIP_DRAW, NULL);
      }
      x_pos += 8;
   }
   {
      LuNumEditModalValue abs_value = widget->cur_value;
      if (abs_value < 0)
         abs_value = -abs_value;
      for(u8 i = 0; i < widget->digit_count; ++i) {
         u8 digit_value;
         {
            u8  power_index = widget->digit_count - i - 1;
            u32 power       = powers_of_ten[power_index];
            if (abs_value < power) {
               buffer[0] = CHAR_0;
            } else {
               LuNumEditModalValue upper_part = abs_value / power;
               digit_value = upper_part - (upper_part / 10 * 10);
            }
         }
         buffer[0] = CHAR_0 + digit_value;
         AddTextPrinterParameterized(widget->window_id, FONT_NORMAL, buffer, x_pos, 1, TEXT_SKIP_DRAW, NULL);
         
         x_pos += 8;
      }
   }
   CopyWindowToVram(widget->window_id, COPYWIN_GFX);
}

extern void DestroyNumEditModal(struct LuNumEditModal* widget) {
   DebugPrintf("[LuNumEditModal] Destroying widget at %08X...", widget);
   
   widget->active = FALSE;
   if (widget->task_id != TASK_NONE) {
      DebugPrintf("[LuNumEditModal] Destroying task...");
      DestroyTask(widget->task_id);
      widget->task_id = TASK_NONE;
   }
   if (widget->window_id != WINDOW_NONE) {
      DebugPrintf("[LuNumEditModal] Clearing BG tiles...");
      const struct Window* win = &gWindows[widget->window_id];
      FillBgTilemapBufferRect(
         win->window.bg,
         0,
         win->window.tilemapLeft - 1,
         win->window.tilemapTop  - 1,
         win->window.width       + 2,
         win->window.height      + 2,
         win->window.paletteNum
      );
      CopyBgTilemapBufferToVram(win->window.bg);
      
      DebugPrintf("[LuNumEditModal] Destroying window...");
      RemoveWindow(widget->window_id);
      widget->window_id = WINDOW_NONE;
   }
   if (widget->cursor_sprite_id != SPRITE_NONE) {
      DebugPrintf("[LuNumEditModal] Destroying cursor sprite...");
      DestroySprite(&gSprites[widget->cursor_sprite_id]);
      widget->cursor_sprite_id = SPRITE_NONE;
   }
   if (widget->heap_free_on_destroy) {
      DebugPrintf("[LuNumEditModal] Heap-freeing...");
      widget->heap_free_on_destroy = FALSE;
      Free(widget);
   }
}

extern void FireAndForgetNumEditModal(const struct LuNumEditModalInitParams* params) {
   //
   DebugPrintf("[LuNumEditModal] Heap-allocating and fire-and-forgetting...");
   AGB_ASSERT(params->use_task);
   AGB_ASSERT(!!params->callback);
   struct LuNumEditModal* widget = AllocZeroed(sizeof(struct LuNumEditModal));
   AGB_ASSERT(widget != NULL);
   InitNumEditModal(widget, params);
   AGB_ASSERT(widget->task_id != TASK_NONE);
   widget->heap_free_on_destroy = TRUE;
   //*/
   
   /*//
   AGB_ASSERT(!params->use_task);
   AGB_ASSERT(!!params->callback);
   
   u8 task_id = CreateTask(NULL, 50);
   AGB_ASSERT(task_id != TASK_NONE);
   
   struct Task*           task   = &gTasks[task_id];
   struct LuNumEditModal* widget = (struct LuNumEditModal*) &task->data[2];
   InitNumEditModal(widget, params);
   widget->task_id = task_id;
   SetWordTaskArg(widget->task_id, 0, (u32)widget);
   //*/
}