#include "hicolor/hicolor.h"
#include "gba/defines.h" // DISPLAY_HEIGHT, EWRAM_DATA
#include "gba/isagbprint.h"
#include "lu/c.h"
#include "lu/macros/ARRAY_COUNT.h"

#define HICOLOR_PALETTE_COUNT          HICOLOR_MAX_SPRITES
#define INVALID_HICOLOR_PALETTE_INDEX  HICOLOR_PALETTE_COUNT

typedef u8 HiColorPaletteIndex;

struct HiColorSpriteState {
   struct Sprite* sprite;
   HiColorPaletteIndex hicolor_palette_index;
   u8 screen_y_top;
   u8 screen_y_bottom;
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
      
      // The VRAM indices to be used for each HiColor palette for the next 
      // frame.
      u8 vram_indices[HICOLOR_PALETTE_COUNT];
   } palettes;
   struct HiColorSpriteState[HICOLOR_MAX_SPRITES] sprite_state;
};

ALIGNED(4) EWRAM_DATA struct HiColorState sHiColorState = {0};

extern void HiColor_Init() {
   CpuFill16(HICOLOR_PALETTE_TAG_NONE, sHiColorState.palettes.tags, sizeof(sHiColorState.palettes.tags));
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
      CpuCopy(cpu_palette_src, sHiColorState.palettes.baseline[hicolor_id], 16);
      CpuCopy(cpu_palette_src, sHiColorState.palettes.blending[hicolor_id], 16);
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
//
extern bool8 HiColor_RegisterSprite(struct Sprite* sprite, HiColorPaletteTag hicolor_palette_tag) {
   AGB_WARNING(sprite->inUse && "Don't register sprites with HiColor if the sprites don't exist!");
   bool8 assigned = FALSE;
   for(u8 i = 0; i < HICOLOR_MAX_SPRITES; ++i) {
      auto state = &sHiColorState.sprite_state;
      if (state->sprite == NULL) {
         state->sprite = sprite;
         
         state->hicolor_palette_index = AllocateHiColorPalette(hicolor_palette_tag, sprite->oam.paletteNum);
         state->screen_y_top    = 0;
         state->screen_y_bottom = 0;
         
         assigned = TRUE;
         #ifdef NDEBUG
         break;
         #endif
      }
      AGB_ASSERT(state->sprite != sprite && "Don't register a sprite with HiColor multiple times!");
   }
   return assigned;
}
extern void HiColor_UnregisterSprite(struct Sprite* sprite) {
   AGB_WARNING(sprite->inUse && "Unregister sprites with HiColor before destroying the sprites!");
   for(u8 i = 0; i < HICOLOR_MAX_SPRITES; ++i) {
      auto state = &sHiColorState.sprite_state;
      if (state->sprite != sprite)
         continue;
      state->sprite = NULL;
      HiColorPaletteDecRef(state->hicolor_palette_index);
   }
}

#if 1 // Forward declarations of static functions
   static u8              FindFreePaletteIndex(void);
   static HiColor_Palette PaletteByIndex(u8 index, bool8 blending);
   static u8              HiColorPaletteIndexForSprite(const struct Sprite*);
   static u8              HiColorPaletteIndexForTag(HiColorPaletteTag);
   
   static void UpdateHiColorSpriteStates(void);
#endif

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

#if 1 // Implementations of static functions
   static u8 FindFreePaletteIndex(void) {
      for(u8 i = 0; i < HICOLOR_PALETTE_COUNT; ++i) {
         u8 refcount = sHiColorState.palettes.refcounts[i];
         if (refcount == 0)
            return i;
      }
      return INVALID_HICOLOR_PALETTE_INDEX;
   }
   static HiColor_Palette PaletteByIndex(u8 index, bool8 blending) {
      if (blending)
         return sHiColorState.palettes.blending[index];
      return sHiColorState.palettes.baseline[index];
   }
   static u8 HiColorPaletteIndexForSprite(const struct Sprite* sprite) {
      if (sprite == NULL)
         return INVALID_HICOLOR_PALETTE_INDEX;
      for(int i = 0; i < ARRAY_COUNT(sHiColorState.sprite_state); ++i) {
         auto item = &sHiColorState.sprite_state[i];
         if (item->sprite == sprite)
            return item->hicolor_palette_index;
      }
      return INVALID_HICOLOR_PALETTE_INDEX;
   }
   static u8 HiColorPaletteIndexForTag(HiColorPaletteTag tag) {
      if (tag == HICOLOR_PALETTE_TAG_NONE)
         return INVALID_HICOLOR_PALETTE_INDEX;
      for(int i = 0; i < ARRAY_COUNT(sHiColorState.palettes.tags); ++i)
         if (sHiColorState.palettes.tags[i] == tag)
            return i;
      return INVALID_HICOLOR_PALETTE_INDEX;
   }
   
   EWRAM_DATA struct HiColorSpriteUpdate {
      u8 sprites_per_scanline[DISPLAY_HEIGHT];
      u8 sorted_scanline_list[DISPLAY_HEIGHT];
   } sHiColorSpriteUpdate = {0};
   //
   static void InsertionSortU8(u8* array, u8 size) {
      for(u16 i = 1; i < size; ++i) {
         u8  x = array[i];
         s16 j = i;
         for(; j > 0 && array[j - 1] > x; --j) {
            array[j] = array[j - 1];
         }
         array[j] = x;
      }
   }
   //
   static void UpdateHiColorSpriteStates(void) {
      //
      // First, count how many sprites are on each scanline.
      //
      memset(sHiColorSpriteUpdate.sprites_per_scanline, 0, DISPLAY_HEIGHT);
      //
      u8 nonzero_scanline_count = 0;
      for(u8 i = 0; i < HICOLOR_MAX_SPRITES; ++i) {
         auto mapping = &sHiColorState->sprite_state[i];
         if (!mapping->sprite)
            continue;
         u8 top    = 0;
         u8 bottom = 0;
         {  // TODO: Get sprite bounds
         }
         if (top < 0)
            top = 0;
         else if (top > DISPLAY_HEIGHT) {
            top = bottom = DISPLAY_HEIGHT;
         }
         if (bottom > DISPLAY_HEIGHT) {
            bottom = DISPLAY_HEIGHT;
         }
         mapping->screen_y_top    = top;
         mapping->screen_y_bottom = bottom;
         if (top == bottom) {
            mapping->has_vram_palette_index = TRUE;
            mapping->vram_palette_index     = 0;
            continue;
         }
         mapping->has_vram_palette_index = FALSE;
         
         nonzero_scanline_count + (bottom - top);
         for(u8 y = top; y < bottom; ++y)
            sHiColorSpriteUpdate.sprites_per_scanline[y]++;
      }
      //
      // Next, sort scanlines from most-to-least scanlines.
      //
      {
         u8 count_copied = 0;
         u8 max_count    = 0;
         for(u8 i = 0; i < DISPLAY_HEIGHT; ++i) {
            u8 count = sHiColorSpriteUpdate.sprites_per_scanline[i];
            if (count == 0)
               continue;
            if (count > max_count)
               max_count = count;
            sHiColorSpriteUpdate.sorted_scanline_list[count_copied] = count;
            ++count_copied;
            if (count_copied >= nonzero_scanline_count)
               break;
         }
         for(u8 i = count_copied; i < DISPLAY_HEIGHT; ++i)
            sHiColorSpriteUpdate.sorted_scanline_list[i] = 0;
         
         InsertionSortReverseU8(sHiColorSpriteUpdate.sorted_scanline_list, count_copied);
      }
      //
      // Next, assign VRAM palette IDs.
      //
      memset(sHiColorSpriteUpdate.palettes.vram_indices, 0xFF, HICOLOR_PALETTE_COUNT);
      for(u8 i = 0; i < DISPLAY_HEIGHT; ++i) {
         u8 y = sHiColorSpriteUpdate.sorted_scanline_list[i];
         
         //
         // See which palette colors are already mapped.
         //
         u16 assigned_vram_palettes  = ~sHiColorState.available_vram_palettes;
         u8  sprites_needing_mapping = 0;
         for(u8 j = 0; j < HICOLOR_MAX_SPRITES; ++j) {
            auto info = &sHiColorState->sprite_state[j];
            if (!info->sprite || info->has_vram_palette_index)
               continue;
            if (info->screen_y_top <= y && info->screen_y_bottom > y) {
               u8 hico_index = info->hicolor_palette_index;
               u8 vram_index = sHiColorSpriteUpdate.palettes.vram_indices;
               if (hico_index >= HICOLOR_PALETTE_COUNT)
                  continue;
               if (vram_index == 0xFF)
                  ++sprites_needing_mapping;
               else
                  assigned_vram_palettes |= 1 << vram_index;
            }
         }
         //
         // Map remaining colors.
         //
         if (sprites_needing_mapping > 0) {
            for(u8 j = 0; j < HICOLOR_MAX_SPRITES; ++j) {
               auto info = &sHiColorState->sprite_state[j];
               if (!info->sprite || info->has_vram_palette_index)
                  continue;
               if (info->screen_y_top <= y && info->screen_y_bottom > y) {
                  u8   hico_index = info->hicolor_palette_index;
                  auto vram_i_ptr = &sHiColorSpriteUpdate.palettes.vram_indices;
                  if (hico_index >= HICOLOR_PALETTE_COUNT)
                     continue;
                  if (*vram_i_ptr != 0xFF)
                     continue;
                  u8 vram_index = BitCountRZero16(assigned_vram_palettes);
                  sHiColorSpriteUpdate.palettes.vram_indices[hico_index] = vram_index;
                  assigned_vram_palettes |= 1 << vram_index;
               }
            }
         }
      }
   }
#endif

extern void HiColor_VBlank(void) {
   _Static_assert(0, "TODO");
}
extern void HiColor_HBlank(void) {
   _Static_assert(0, "TODO");
   //
   // The game's vanilla "scanline effect" system doesn't use an h-blank callback 
   // at all. However, it locks us off from DMA0, and it means we'll need our own 
   // buffers to store any data we pre-compute for h-blank.
   //
   // In particular:
   // 
   //  - UpdateHiColorSpriteStates needs to be faster to execute.
   // 
   //     - Possible optimization: once we've sorted scanlines, we loop over each 
   //       scanline. Within each scanline, we first identify which VRAM palette 
   //       indices are already assigned to sprites that overlap the scanlines above 
   //       us; then, we loop again to assign the remaining palettes.
   //
   //       The "identify" step involves looping over every sprite, and seeing which 
   //       ones have Y-coordinates that overlap us. This is only necessary because 
   //       we assign palettes to scanlines non-sequentially. If we processed all 
   //       scanlines from top to bottom, then we could instead:
   //
   //        - Loop over every sprite. If the sprite overlaps the current scanline, 
   //          then: assign its palette a VRAM index if said palette doesn't already 
   //          have one; and update a bitmask of "VRAM palette indices consumed by 
   //          this scanline."
   //
   //        - On the next scanline, use the previous scanline's bitmask of "VRAM 
   //          palette indices consumed by this scanline" to immediately know which 
   //          VRAM palette indices remain available to assign.
   //
   //       This optimization takes advantage of the fact that if a sprite overlaps 
   //       the scanline at (Y), it must necessarily overlap the scanline at (Y-1); 
   //       it's not possible for a sprite to overlap (Y) and (Y-N), for any value 
   //       N != 1, without also overlapping (Y-1). If we process scanlines in order 
   //       contiguously, then we can leverage that to do a lot less looping and 
   //       processing overall.
   //
   //       The only advantage of processing scanlines in descending order of number 
   //       of contained sprites is that we can assign palette indices more reliably, 
   //       but I'm not even fully sure that we *need* that.
   // 
   //  - UpdateHiColorSpriteStates needs to compute data that h-blank can read more 
   //    quickly.
   //     - e.g.
   //        - u16 vram_palette_indices_to_update; // mask
   //        - u8  hicolor_palette_indices[DISPLAY_HEIGHT][16];
   // 
   //  - Said data needs to be double-buffered.
   //
   // gScanlineEffectRegBuffers is 0x3C0 bytes not counting the double-buffering. 
   // If we stored 16 bytes per scanline, we'd be 0xA00 bytes per buffer.
   //
}

_Static_assert(0, "TODO: when looping over sprites for any reason, if we find that a sprite that we have registered isn't `inUse` anymore, then unregister it internally. This can happen if some game system resets all sprites en masse without talking to us (e.g. literally any menu opening). Ideally, our own code using this HiColor system would reset HiColor on exit, so that HiColor doesn't get confused when other game screens reset sprites on entry, but we should still code defensively.");