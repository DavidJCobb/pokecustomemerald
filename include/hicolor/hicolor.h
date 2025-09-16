#ifndef GUARD_HICOLOR
#define GUARD_HICOLOR

#include "gba/types.h"
struct Sprite;

#define HICOLOR_MAX_SPRITES 32

typedef u16 HiColorPaletteTag;
#define HICOLOR_PALETTE_TAG_NONE ((HiColorPaletteTag)0xFFFF)

typedef ALIGNED(2) u16 HiColor_Palette[16];

extern void HiColor_Init();

extern void HiColor_SetAvailableVRAMPalettes(u16 mask);

extern bool8 HiColor_RegisterSprite  (struct Sprite*, HiColorPaletteTag);
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

extern void HiColor_VBlank(void);
extern void HiColor_HBlank(void);

#endif