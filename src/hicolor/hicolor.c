#include "hicolor/hicolor.h"
#include "gba/defines.h" // DISPLAY_HEIGHT, EWRAM_DATA
#include "gba/io_reg.h"
#include "gba/isagbprint.h"
#include "gba/macro.h" // CpuFill16
#include "lu/c.h"
#include "lu/algorithms/bit.h"
#include "lu/macros/ARRAY_COUNT.h"
#include "lu/macros/min_max.h"
#include "global.h" // for transitive include of <string.h>
#include "palette.h"
#include "sprite.h"
#include "lu/c-attr.define.h"

//
// Configuration:
//

// Control whether we use DMA0 or the CpuSet syscall to copy HiColor palettes into 
// VRAM during the h-blank interrupt. The former would make us incompatible with the 
// Game Freak "scanline effect" system.
#define USE_DMA_FOR_HICOLOR_HBLANK   0

#define USE_ASM_FOR_HICOLOR_HBLANK   1

// Control whether and how we check for "zombie" sprites during our CB2 handler, i.e. 
// sprites that were destroyed without being unregistered first. "Eager" detection 
// means that we check for them while mapping HiColor palettes to VRAM, potentially 
// slowing that operation down; "non-eager" detection means we check for them after 
// HiColor-to-VRAM mappings are built.
#define GUARD_AGAINST_ZOMBIE_SPRITES 0 // Check for zombie sprites during our CB2 handler.
#define EAGER_DETECT_ZOMBIE_SPRITES  0 // Check for zombie sprites early.

// Set up each sprite's palette N scanlines early (i.e. allocate palettes for copying 
// into VRAM as if the sprite stretched N pixels further up than it really does). Use 
// this to account for the fact that even with the fastest possible copies during the 
// h-blank interrupt, we may still be a scanline late.
#define PALETTES_EARLY_BY_SCANLINES  2

// Any sprites with a Y-coordinate below this value will be treated as if they extend 
// to the top of the screen, i.e. h-blank will set up their palettes as early as it 
// possibly can. As of this writing, this doesn't seem to be needed in tests, but it's 
// been left here in case anything is slow enough that we can't properly handle the 
// topmost scanlines on-screen. That was happening in early development, and I can't 
// say I'm altogether sure how (and therefore whether) I (truly) fixed it.
#define LOST_CAUSE_SCANLINES 0

// Control whether we re-check sprites' sizes per frame. As of this writing, what we 
// check is, specifically, whether the sprite's affine transform mode or subsprite 
// table index have changed.
#define RECHECK_SPRITE_SIZES_PER_FRAME 0

// As of this writing, we don't quite manage to run once per frame. Close, but not 
// quite. If this setting is non-zero, and the update function is called for the same 
// buffer twice in a row, it skips the second update. If this setting is zero, then 
// we skip the (minor) overhead of even checking that.
#define SKIP_UPDATES_IF_ONLY_BARELY_STALE 0

//
// End of configuration.
//

#define HICOLOR_PALETTE_COUNT          HICOLOR_MAX_SPRITES
#define INVALID_HICOLOR_PALETTE_INDEX  HICOLOR_PALETTE_COUNT

#define COLORS_PER_PALETTE 16
#define VRAM_PALETTE_COUNT 16

#define clamp(_val, _min, _max) \
   ({ \
      const auto v = (_val); \
      const auto a = (_min); \
      const auto b = (_max); \
      v < a ? a : (v > b ? b : v); \
   })

typedef u8 HiColorPaletteIndex;
struct HiColorSpriteState;

#if 1 // Forward declarations of static functions
   static HiColorPaletteIndex FindFreePaletteIndex(void);
   static u16*                PaletteByIndex(HiColorPaletteIndex index, bool8 blending);
   static HiColorPaletteIndex HiColorPaletteIndexForSprite(const struct Sprite*);
   static HiColorPaletteIndex HiColorPaletteIndexForTag(HiColorPaletteTag);
   
   static void RecalcSpriteSize(struct HiColorSpriteState*, bool8 force);
   static void ComputeSpriteBounds(struct HiColorSpriteState*, bool8 force);
   
   static void UpdateHiColorSpriteStates(void);
#endif

struct OamDimensions {
   u8 width;
   u8 height;
};
static const struct OamDimensions sOamDimensions[3][4] = {
   [ST_OAM_SQUARE] = {
      [SPRITE_SIZE(8x8)]   = {  8,  8 },
      [SPRITE_SIZE(16x16)] = { 16, 16 },
      [SPRITE_SIZE(32x32)] = { 32, 32 },
      [SPRITE_SIZE(64x64)] = { 64, 64 },
   },
   [ST_OAM_H_RECTANGLE] = {
      [SPRITE_SIZE(16x8)]  = { 16,  8 },
      [SPRITE_SIZE(32x8)]  = { 32,  8 },
      [SPRITE_SIZE(32x16)] = { 32, 16 },
      [SPRITE_SIZE(64x32)] = { 64, 32 },
    },
   [ST_OAM_V_RECTANGLE] = {
      [SPRITE_SIZE(8x16)]  = {  8, 16 },
      [SPRITE_SIZE(8x32)]  = {  8, 32 },
      [SPRITE_SIZE(16x32)] = { 16, 32 },
      [SPRITE_SIZE(32x64)] = { 32, 64 },
   },
};

struct HiColorSpriteState {
   struct Sprite* sprite;
   HiColorPaletteIndex hicolor_palette_index;
   struct {
      s8    y_min_offset;
      s8    y_max_offset;
      #if RECHECK_SPRITE_SIZES_PER_FRAME
      u8    subsprite_table_index;
      u8    affine_mode : 2;
      #endif
   } cached_size;
   struct {
      u8 top;
      u8 bottom;
   } screen_bounds;
};

ALIGNED(4) typedef HiColorPaletteIndex HiColorPaletteIndicesPerScanline [VRAM_PALETTE_COUNT];
struct HiColorPrecomputedHBlankData {
   bool8 live_buffer_index;
   bool8 staging_in_progress;
   #if SKIP_UPDATES_IF_ONLY_BARELY_STALE
   u8    last_updated_buffer;
   u8    frames_presented_since_last_staging;
   #endif
   ALIGNED(4) HiColorPaletteIndicesPerScanline hicolor_palettes_per_scanline[2][DISPLAY_HEIGHT];
};

struct HiColorState {
   u16 available_vram_palettes;
   struct {
      HiColor_Palette baseline[HICOLOR_PALETTE_COUNT];
      HiColor_Palette blending[HICOLOR_PALETTE_COUNT];
      
      // Refcounts for HiColor palettes.
      u8 refcounts[HICOLOR_PALETTE_COUNT];
      
      // The tag with which a given HiColor palette was registered.
      HiColorPaletteTag tags[HICOLOR_PALETTE_COUNT];
   } palettes;
   struct HiColorSpriteState sprite_state[HICOLOR_MAX_SPRITES];
   
   struct HiColorPrecomputedHBlankData hblank_state;
};

ALIGNED(4) EWRAM_DATA static struct HiColorState sHiColorState = {0};

extern void HiColor_Init(void) {
   HiColor_Reset();
}
extern void HiColor_Reset(void) {
   sHiColorState.available_vram_palettes          = 0;
   sHiColorState.hblank_state.live_buffer_index   = 1;
   #if SKIP_UPDATES_IF_ONLY_BARELY_STALE
   sHiColorState.hblank_state.last_updated_buffer = 1;
   #endif
   
   memset(
      sHiColorState.palettes.refcounts,
      0,
      sizeof(sHiColorState.palettes.refcounts)
   );
   CpuFill16(HICOLOR_PALETTE_TAG_NONE, sHiColorState.palettes.tags, sizeof(sHiColorState.palettes.tags));
   
   for(int i = 0; i < HICOLOR_MAX_SPRITES; ++i) {
      auto state = &sHiColorState.sprite_state[i];
      state->sprite                = NULL;
      state->hicolor_palette_index = INVALID_HICOLOR_PALETTE_INDEX;
   }
   
   memset(
      sHiColorState.hblank_state.hicolor_palettes_per_scanline,
      0xFF,
      sizeof(sHiColorState.hblank_state.hicolor_palettes_per_scanline)
   );
}

extern void HiColor_SetAvailableVRAMPalettes(u16 mask) {
   sHiColorState.available_vram_palettes = mask;
}

static u8 AllocateHiColorPalette(HiColorPaletteTag tag, u8 copy_locolor_palette_id) {
   if (tag != HICOLOR_PALETTE_TAG_NONE) {
      //
      // See if there's an existing HiColor palette we can reuse.
      //
      for(u8 i = 0; i < HICOLOR_PALETTE_COUNT; ++i) {
         if (sHiColorState.palettes.refcounts[i] == 0)
            continue;
         if (sHiColorState.palettes.tags[i] == tag) {
            ++sHiColorState.palettes.refcounts[i];
            return i;
         }
      }
   }
   u8 hicolor_id = FindFreePaletteIndex();
   if (hicolor_id == INVALID_HICOLOR_PALETTE_INDEX)
      return hicolor_id;
   
   ++sHiColorState.palettes.refcounts[hicolor_id];
   sHiColorState.palettes.tags[hicolor_id] = tag;
   if (copy_locolor_palette_id < 16) {
      u16* cpu_palette_src = &gPlttBufferUnfaded[PLTT_OFFSET_4BPP(copy_locolor_palette_id)];
      CpuCopy16(cpu_palette_src, sHiColorState.palettes.baseline[hicolor_id], 16);
      CpuCopy16(cpu_palette_src, sHiColorState.palettes.blending[hicolor_id], 16);
   }
   return hicolor_id;
}
static void HiColorPaletteDecRef(u8 hicolor_id) {
   if (hicolor_id >= HICOLOR_PALETTE_COUNT)
      return;
   AGB_ASSERT(sHiColorState.palettes.refcounts[hicolor_id] > 0);
   if (--sHiColorState.palettes.refcounts[hicolor_id] == 0) {
      sHiColorState.palettes.tags[hicolor_id] = HICOLOR_PALETTE_TAG_NONE;
   }
}
static void DestroySpriteState(struct HiColorSpriteState* state) {
   state->sprite = NULL;
   HiColorPaletteDecRef(state->hicolor_palette_index);
   state->screen_bounds.top    = 0;
   state->screen_bounds.bottom = 0;
}
//
extern bool8 HiColor_RegisterSprite(struct Sprite* sprite, HiColorPaletteTag hicolor_palette_tag, bool8 palette_already_loaded) {
   AGB_ASSERT(sprite != NULL && "[HiColor_RegisterSprite] Don't register a null sprite pointer!");
   AGB_ASSERT(sprite->inUse && "[HiColor_RegisterSprite] Don't register sprites with HiColor if the sprites don't exist!");
   bool8 assigned = FALSE;
   u8    i        = 0;
   for(; i < HICOLOR_MAX_SPRITES; ++i) {
      auto state = &sHiColorState.sprite_state[i];
      if (state->sprite == NULL) {
         state->sprite = sprite;
         
         state->hicolor_palette_index = AllocateHiColorPalette(hicolor_palette_tag, palette_already_loaded ? sprite->oam.paletteNum : 0xFF);
         #ifndef NDEBUG
            if (state->hicolor_palette_index == INVALID_HICOLOR_PALETTE_INDEX) {
               #if HICOLOR_PALETTE_COUNT < HICOLOR_MAX_SPRITES
                  DebugPrintf("[HiColor_RegisterSprite] WARNING: Failed to allocate a HiColor palette for sprite ID %u. Nonetheless, the sprite has been registered.", (u32)sprite / sizeof(struct Sprite));
               #else
                  AGB_ASSERT(FALSE && "[HiColor_RegisterSprite] Somehow we were able to register a sprite, but unable to allocate a palette for it. We have room for exactly as many palettes as the number of sprites that can be registered, so this can only happen if our internal systems fail to free palettes when they're no longer in use.");
               #endif
            }
         #endif
         RecalcSpriteSize(state, TRUE);
         
         assigned = TRUE;
         break;
      }
      AGB_ASSERT(state->sprite != sprite && "[HiColor_RegisterSprite] Don't register a sprite with HiColor multiple times!");
   }
   #ifndef NDEBUG
      if (assigned) {
         for(++i; i < HICOLOR_MAX_SPRITES; ++i) {
            auto state = &sHiColorState.sprite_state[i];
            AGB_ASSERT(state->sprite != sprite && "[HiColor_RegisterSprite] Don't register a sprite with HiColor multiple times!");
         }
      }
   #endif
   return assigned;
}
extern void HiColor_UnregisterSprite(struct Sprite* sprite) {
   AGB_WARNING(sprite->inUse && "Unregister sprites with HiColor before destroying the sprites!");
   for(u8 i = 0; i < HICOLOR_MAX_SPRITES; ++i) {
      auto state = &sHiColorState.sprite_state[i];
      if (state->sprite != sprite)
         continue;
      DestroySpriteState(state);
      return;
   }
}

extern const u16* HiColor_GetBaselinePaletteBySprite(const struct Sprite* sprite) {
   u8 hicolor_id = HiColorPaletteIndexForSprite(sprite);
   if (hicolor_id == INVALID_HICOLOR_PALETTE_INDEX)
      return NULL;
   return sHiColorState.palettes.baseline[hicolor_id];
}
extern u16* HiColor_GetBlendingPaletteBySprite(const struct Sprite* sprite) {
   u8 hicolor_id = HiColorPaletteIndexForSprite(sprite);
   if (hicolor_id == INVALID_HICOLOR_PALETTE_INDEX)
      return NULL;
   return sHiColorState.palettes.blending[hicolor_id];
}
extern const u16* HiColor_GetBaselinePaletteByTag(HiColorPaletteTag tag) {
   u8 hicolor_id = HiColorPaletteIndexForTag(tag);
   if (hicolor_id == INVALID_HICOLOR_PALETTE_INDEX)
      return NULL;
   return sHiColorState.palettes.baseline[hicolor_id];
}
extern u16* HiColor_GetBlendingPaletteByTag(HiColorPaletteTag tag) {
   u8 hicolor_id = HiColorPaletteIndexForTag(tag);
   if (hicolor_id == INVALID_HICOLOR_PALETTE_INDEX)
      return NULL;
   return sHiColorState.palettes.blending[hicolor_id];
}

extern void HiColor_OverwritePaletteByTag_4ByteAlignedSrc(HiColorPaletteTag tag, const u16 colors[static COLORS_PER_PALETTE]) {
   AGB_ASSERT(colors != NULL);
   AGB_ASSERT(tag    != HICOLOR_PALETTE_TAG_NONE);
   AGB_ASSERT(((u32)&colors[0] % 4) == 0 && "Don't call the \"aligned\" version of HiColor_OverwritePaletteByTag with a color array that isn't 4-byte-aligned!");
   HiColorPaletteIndex hicolor_id = HiColorPaletteIndexForTag(tag);
   if (hicolor_id == INVALID_HICOLOR_PALETTE_INDEX) {
      DebugPrintf("[HiColor_OverwritePaletteByTag_4ByteAlignedSrc] Unable to overwrite HiColor palette with tag 0x%04X. No sprite was registered with that tag.", tag);
      return;
   }
   CpuFastSet(
      colors,
      sHiColorState.palettes.baseline[hicolor_id],
      PLTT_SIZE_4BPP / sizeof(u32)
   );
   CpuFastSet(
      colors,
      sHiColorState.palettes.blending[hicolor_id],
      PLTT_SIZE_4BPP / sizeof(u32)
   );
   DebugPrintf(
      "[HiColor_OverwritePaletteByTag_4ByteAlignedSrc] Overwrote palette with tag 0x%04X (HiColor palette ID %u)",
      tag,
      hicolor_id
   );
}
extern void HiColor_OverwritePaletteByTag(HiColorPaletteTag tag, const u16 colors[static COLORS_PER_PALETTE]) {
   AGB_ASSERT(colors != NULL);
   AGB_ASSERT(tag    != HICOLOR_PALETTE_TAG_NONE);
   HiColorPaletteIndex hicolor_id = HiColorPaletteIndexForTag(tag);
   if (hicolor_id == INVALID_HICOLOR_PALETTE_INDEX) {
      DebugPrintf("[HiColor_OverwritePaletteByTag] Unable to overwrite HiColor palette with tag 0x%04X. No sprite was registered with that tag.", tag);
      return;
   }
   DmaCopy16(3, colors, sHiColorState.palettes.baseline[hicolor_id], PLTT_SIZE);
   DmaCopy16(3, colors, sHiColorState.palettes.blending[hicolor_id], PLTT_SIZE);
}

#if 1 // Implementations of static functions
   static HiColorPaletteIndex FindFreePaletteIndex(void) {
      for(HiColorPaletteIndex i = 0; i < HICOLOR_PALETTE_COUNT; ++i) {
         u8 refcount = sHiColorState.palettes.refcounts[i];
         if (refcount == 0)
            return i;
      }
      return INVALID_HICOLOR_PALETTE_INDEX;
   }
   static u16* PaletteByIndex(HiColorPaletteIndex index, bool8 blending) {
      if (blending)
         return sHiColorState.palettes.blending[index];
      return sHiColorState.palettes.baseline[index];
   }
   static HiColorPaletteIndex HiColorPaletteIndexForSprite(const struct Sprite* sprite) {
      if (sprite == NULL)
         return INVALID_HICOLOR_PALETTE_INDEX;
      for(int i = 0; i < ARRAY_COUNT(sHiColorState.sprite_state); ++i) {
         auto item = &sHiColorState.sprite_state[i];
         if (item->sprite == sprite)
            return item->hicolor_palette_index;
      }
      return INVALID_HICOLOR_PALETTE_INDEX;
   }
   static HiColorPaletteIndex HiColorPaletteIndexForTag(HiColorPaletteTag tag) {
      if (tag == HICOLOR_PALETTE_TAG_NONE)
         return INVALID_HICOLOR_PALETTE_INDEX;
      for(HiColorPaletteIndex i = 0; i < ARRAY_COUNT(sHiColorState.palettes.tags); ++i)
         if (sHiColorState.palettes.tags[i] == tag)
            return i;
      return INVALID_HICOLOR_PALETTE_INDEX;
   }
   
   static void RecalcSpriteSize(struct HiColorSpriteState* state, bool8 force) {
      auto sprite = state->sprite;
      if (!force) {
         #if RECHECK_SPRITE_SIZES_PER_FRAME
            bool8 unchanged = TRUE;
            //
            if (sprite->subspriteTableNum != state->cached_size.subsprite_table_index)
               unchanged = FALSE;
            if (sprite->oam.affineMode != state->cached_size.affine_mode)
               unchanged = FALSE;
            //
            if (unchanged)
               return;
         #else
            return;
         #endif
      }
      
      #if RECHECK_SPRITE_SIZES_PER_FRAME
         state->cached_size.affine_mode = sprite->oam.affineMode;
      #endif
      if (!sprite->subspriteTables) {
         //
         // Non-subsprited sprites use their coordinates as a centerpoint.
         //
         u8 height = sOamDimensions[sprite->oam.shape][sprite->oam.size].height;
         if (sprite->oam.affineMode == ST_OAM_AFFINE_DOUBLE)
            height *= 2;
         
         height /= 2;
         if (height > 128)
            height = 128;
         state->cached_size.y_min_offset = -(s16)height;
         if (height == 128)
            --height;
         state->cached_size.y_max_offset = height;
         return;
      }
      
      //
      // Subsprited sprites consist of multiple fragments, each independently 
      // defining the offset of its top-left corner from the sprite's coordinates.
      //
      
      auto which = (int)sprite->subspriteTableNum; // __auto_type chokes on bitfields...
      auto table = &sprite->subspriteTables[sprite->subspriteTableNum];
      auto count = table->subspriteCount;
      #if RECHECK_SPRITE_SIZES_PER_FRAME
         state->cached_size.subsprite_table_index = which;
      #endif
      //
      s16 min_y =  32767;
      s16 max_y = -32768;
      for(int i = 0; i < count; ++i) {
         auto subsprite = &table->subsprites[i];
         u16 y = subsprite->y;
         s8  h = sOamDimensions[subsprite->shape][subsprite->size].height;
         
         u16 bottom = y + h;
         if (y < min_y)
            min_y = y;
         if (bottom > max_y)
            max_y = bottom;
      }
      
      state->cached_size.y_min_offset = clamp(min_y, -128, 127);
      state->cached_size.y_max_offset = clamp(max_y, -128, 127);
   }
   static void ComputeSpriteBounds(struct HiColorSpriteState* state, bool8 force) {
      auto sprite = state->sprite;
      if (sprite->invisible) {
         state->screen_bounds.top    = 0;
         state->screen_bounds.bottom = 0;
         return;
      }
      #if RECHECK_SPRITE_SIZES_PER_FRAME
         RecalcSpriteSize(state, force);
      #endif
      
      s16 y = sprite->y + sprite->y2;
      state->screen_bounds.top    = clamp(y + state->cached_size.y_min_offset, 0, DISPLAY_HEIGHT);
      state->screen_bounds.bottom = clamp(y + state->cached_size.y_max_offset, 0, DISPLAY_HEIGHT);
   }
   
   static void UpdateHiColorSpriteStates(void) {
      sHiColorState.hblank_state.staging_in_progress = TRUE;
      u8 which_is_staging = sHiColorState.hblank_state.live_buffer_index ^ 1;
      #if SKIP_UPDATES_IF_ONLY_BARELY_STALE
         if (
            sHiColorState.hblank_state.last_updated_buffer == which_is_staging
         && sHiColorState.hblank_state.frames_presented_since_last_staging == 0
         ) {
            sHiColorState.hblank_state.staging_in_progress = FALSE;
            return;
         }
      #endif
DebugPrintf("[UpdateHiColorSpriteStates] Staging buffer is #%d.", which_is_staging);

      typedef u8 ScanlineData[VRAM_PALETTE_COUNT];

      ScanlineData* scanline_list = &sHiColorState.hblank_state.hicolor_palettes_per_scanline[which_is_staging][0];
      CpuFill32(0xFFFFFFFF, scanline_list, sizeof(ScanlineData) * DISPLAY_HEIGHT);
      
      u8 first_permitted_palette_index = BitCountRZero16(sHiColorState.available_vram_palettes);
      
      for(u8 i = 0; i < HICOLOR_MAX_SPRITES; ++i) {
         auto state = &sHiColorState.sprite_state[i];
         if (!state->sprite)
            continue;
         #if GUARD_AGAINST_ZOMBIE_SPRITES && EAGER_DETECT_ZOMBIE_SPRITES
            if (!state->sprite->inUse) {
               //
               // Sprite was destroyed, but we were never told about it.
               //
               DestroySpriteState(state);
               continue;
            }
         #endif
         ComputeSpriteBounds(state, FALSE);
         if (state->screen_bounds.top == state->screen_bounds.bottom)
            continue;
         const u8 hicolor_index = state->hicolor_palette_index;
         #if HICOLOR_PALETTE_COUNT < HICOLOR_MAX_SPRITES
            if (hicolor_index >= HICOLOR_PALETTE_COUNT)
               continue;
         #endif
         
         u8 vram_index = 0xFF;
         u8 y          = state->screen_bounds.top;
         if (y < LOST_CAUSE_SCANLINES)
            y = 0;
         else if (y >= PALETTES_EARLY_BY_SCANLINES)
            y -= PALETTES_EARLY_BY_SCANLINES;
         {
            u8* scanline = scanline_list[y];
            
            u8  j   = first_permitted_palette_index;
            u16 bit = 1 << j;
            for(; j < VRAM_PALETTE_COUNT; ++j, bit <<= 1) {
               if (scanline[j] != 0xFF)
                  continue;
               if ((sHiColorState.available_vram_palettes & bit) == 0)
                  continue;
               vram_index = j;
               scanline[j] = hicolor_index;
               break;
            }
         }
         if (vram_index != 0xFF) {
            state->sprite->oam.paletteNum = vram_index;
            for(++y; y < state->screen_bounds.bottom; ++y) {
               scanline_list[y][vram_index] = hicolor_index;
            }
         }
      }
      #if SKIP_UPDATES_IF_ONLY_BARELY_STALE
         sHiColorState.hblank_state.last_updated_buffer = which_is_staging;
      #endif
      sHiColorState.hblank_state.staging_in_progress = FALSE;
      #if SKIP_UPDATES_IF_ONLY_BARELY_STALE
         sHiColorState.hblank_state.frames_presented_since_last_staging = 0;
      #endif
DebugPrintf("[UpdateHiColorSpriteStates] Done.");
      #if GUARD_AGAINST_ZOMBIE_SPRITES && EAGER_DETECT_ZOMBIE_SPRITES == 0
         for(u8 i = 0; i < HICOLOR_MAX_SPRITES; ++i) {
            auto state = &sHiColorState.sprite_state[i];
            if (!state->sprite || state->sprite->inUse)
               continue;
            //
            // Sprite was destroyed, but we were never told about it.
            //
            DestroySpriteState(state);
         }
      #endif
   }
#endif

extern void HiColor_CB2(void) {
   UpdateHiColorSpriteStates();
}

extern void HiColor_VBlank(void) {
   if (sHiColorState.hblank_state.staging_in_progress) {
      #if SKIP_UPDATES_IF_ONLY_BARELY_STALE
         ++sHiColorState.hblank_state.frames_presented_since_last_staging;
      #endif
      return;
   }
   sHiColorState.hblank_state.live_buffer_index ^= 1;
   DebugPrintf("[HiColor_VBlank] Switched live buffer to #%d.", sHiColorState.hblank_state.live_buffer_index);
}

#if USE_ASM_FOR_HICOLOR_HBLANK
   extern void HiColor_HBlank_Asm(const void*, void*) GNU_ATTR(naked);
#endif
extern void HiColor_HBlank(void) GNU_ATTR(optimize(3)) {
   uint_fast8_t vcount = REG_VCOUNT;
   if (vcount >= DISPLAY_HEIGHT)
      return;
   
   #if USE_ASM_FOR_HICOLOR_HBLANK
      const u8* scanline = sHiColorState.hblank_state.hicolor_palettes_per_scanline[sHiColorState.hblank_state.live_buffer_index][vcount];
      auto      blending = sHiColorState.palettes.blending;
      HiColor_HBlank_Asm(scanline, blending);
   #else
      const u8* scanline = sHiColorState.hblank_state.hicolor_palettes_per_scanline[sHiColorState.hblank_state.live_buffer_index][vcount];
      uint_fast8_t pal;
      #if USE_DMA_FOR_HICOLOR_HBLANK
         #define LOOP_BODY(n)     \
            do {                  \
               pal = scanline[n]; \
               if (pal != 0xFF)   \
                  DmaCopy16(0, sHiColorState.palettes.blending[pal], (void*)(OBJ_PLTT + PLTT_OFFSET_4BPP(n)), PLTT_SIZE_4BPP); \
            } while (0)
      #else
         #define LOOP_BODY(n)     \
            do {                  \
               pal = scanline[n]; \
               if (pal != 0xFF)   \
                  CpuFastSet(sHiColorState.palettes.blending[pal], (void*)(OBJ_PLTT + PLTT_OFFSET_4BPP(n)), VRAM_PALETTE_COUNT * sizeof(u16) / sizeof(u32)); \
            } while (0)
      #endif
      
      LOOP_BODY(0);
      LOOP_BODY(1);
      LOOP_BODY(2);
      LOOP_BODY(3);
      LOOP_BODY(4);
      LOOP_BODY(5);
      LOOP_BODY(6);
      LOOP_BODY(7);
      LOOP_BODY(8);
      LOOP_BODY(9);
      LOOP_BODY(10);
      LOOP_BODY(11);
      LOOP_BODY(12);
      LOOP_BODY(13);
      LOOP_BODY(14);
      LOOP_BODY(15);
      
      #undef LOOP_BODY
   #endif
}
