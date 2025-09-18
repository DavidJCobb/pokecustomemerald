#ifndef GUARD_HICOLOR
#define GUARD_HICOLOR

#include "gba/defines.h"
#include "gba/types.h"
struct Sprite;

#define HICOLOR_MAX_SPRITES 32

typedef u16 HiColorPaletteTag;
#define HICOLOR_PALETTE_TAG_NONE ((HiColorPaletteTag)0xFFFF)

ALIGNED(4) typedef u16 HiColor_Palette[16];

extern void HiColor_Init(void);
extern void HiColor_Reset(void);

extern void HiColor_SetAvailableVRAMPalettes(u16 mask);

extern bool8 HiColor_RegisterSprite  (struct Sprite*, HiColorPaletteTag, bool8 palette_already_loaded);
extern void  HiColor_UnregisterSprite(struct Sprite*);

//
// Accessors for a sprite's HiColor palettes. The "baseline" palette 
// is the palette without any blending or other effects, while the 
// "blending" palette is what you'd want to modify to do blending or 
// other effects.
//
// These are allowed to return NULL.
//
extern const u16* HiColor_GetBaselinePaletteBySprite(const struct Sprite*);
extern u16* HiColor_GetBlendingPaletteBySprite(const struct Sprite*);
extern const u16* HiColor_GetBaselinePaletteByTag(HiColorPaletteTag);
extern u16* HiColor_GetBlendingPaletteByTag(HiColorPaletteTag);

extern void HiColor_OverwritePaletteByTag_4ByteAlignedSrc(HiColorPaletteTag, const u16[static 16]);
extern void HiColor_OverwritePaletteByTag(HiColorPaletteTag, const u16[static 16]);

// Run this after `AnimateSprites` and before `BuildOamBuffer`.
extern void HiColor_CB2(void);

extern void HiColor_VBlank(void);
extern void HiColor_HBlank(void);

#endif