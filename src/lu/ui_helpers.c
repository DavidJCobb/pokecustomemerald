#include "lu/ui_helpers.h"

#include "bg.h"
#include "gpu_regs.h"
#include "menu_helpers.h" // ResetAllBgsCoordinates
#include "palette.h" // ResetPaletteFade
#include "scanline_effect.h" // ScanlineEffect_Stop
#include "sprite.h" // FreeAllSpritePalettes, ResetSpriteData
#include "text_window.h" // GetWindowFrameTilesPal
#include "window.h"

void LuUI_ResetBackgroundsAndVRAM(void) {
   //
   // Clear VRAM, OAM, and palettes, and reset some VRAM data. We're basically 
   // doing memset to write 0s over it all. Each background layer has its tiles 
   // set to use tileset entry 0, palette 0.
   //
   // NOTE: Some Game Freak menus use  `ResetVramOamAndBgCntRegs`, defined in 
   // `menu_helpers.c`, for this task. That uses CPU fills rather than DMA fills. 
   // I don't know when it'd be appropriate to prefer CPU fills over DMA fills.
   //
   DmaClearLarge16(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
   DmaClear32(3, OAM, OAM_SIZE);
   DmaClear16(3, PLTT, PLTT_SIZE);
   SetGpuReg(REG_OFFSET_DISPCNT, 0);
   ResetBgsAndClearDma3BusyFlags(0);
   //
   // Reset all four background layers' positions.
   //
   ResetAllBgsCoordinates();
   
   HideBg(0);
   HideBg(1);
   HideBg(2);
   HideBg(3);
   
   //
   // Reset the color effect.
   //
   SetGpuReg(REG_OFFSET_BLDCNT, 0);
}
void LuUI_ResetSpritesAndEffects(void) {
   // The GBA maintains two sets of palettes: one for BG tiles and one for 
   // OBJ graphics (sprites). Game Freak's engine, however, maintains four 
   // sets of palettes in normal RAM: two "source" sets and two blended 
   // sets. This lets them set up full-screen color blending. We are here 
   // resetting that RAM state and clearing color blend effects.
   ResetPaletteFade();
   
   // The GBA draws one row of pixels (a scanline) at a time. DMA can be 
   // used to write to hardware registers right as the GBA finishes one 
   // row and before it starts the next row. This means that parameters 
   // like the BG layer offsets can be varied on a per-screen-pixel-row 
   // basis, allowing for "wavy" screen distortions. It seems that Game 
   // Freak wants to make absolutely sure that these effects aren't being 
   // shown when their menus open; even menus that open from places which 
   // never use scanline effects will still call this function.
   ScanlineEffect_Stop();
   
   // Sprites are just graphics shown on the OBJ layer, generally paired 
   // with OAM data to control how they display. Game Freak has built an 
   // abstraction layer on top of this to add additional features:
   //
   //  - Sprites that run a callback function on every frame.
   //
   //  - A "sprite," as considered by Game Freak's library, is decoupled 
   //    from its actual graphics and palette. The latter are identified 
   //    by an arbitrary integer "tag." This makes it easier to manage 
   //    loaded resources: when loading a sprite palette, for example, 
   //    Game Freak's code will check if a palette with that tag is in 
   //    place already.
   //
   // We are here resetting sprite state in both RAM and VRAM+OAM.
   ResetSpriteData();
   FreeAllSpritePalettes();
}

void LuUI_LoadPlayerWindowFrame(u8 bg_layer, u8 bg_palette_index, u16 destination_tile_id) {
   const struct TilesPal* info = GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType);
   
   LoadBgTiles(bg_layer, info->tiles, 0x20 * 9, destination_tile_id);
   LoadPalette(info->pal, BG_PLTT_ID(bg_palette_index), PLTT_SIZE_4BPP);
}

// =========================================================================================

void LuUI_DrawWindowFrame(
   u8  bg_layer,
   u16 dialog_frame_first_tile_id,
   u8  palette_id,
   u8  inner_x, // measured in tiles, not pixels
   u8  inner_y, // measured in tiles, not pixels
   u8  inner_w, // measured in tiles, not pixels
   u8  inner_h  // measured in tiles, not pixels
) {
   u8 l = inner_x - 1;
   u8 r = inner_x + inner_w;
   u8 t = inner_y - 1;
   u8 b = inner_y + inner_h;
   
   // C: corner
   // E: edge
   u8 tile_c_tl = dialog_frame_first_tile_id;
   u8 tile_e_t  = dialog_frame_first_tile_id + 1;
   u8 tile_c_tr = dialog_frame_first_tile_id + 2;
   u8 tile_e_l  = dialog_frame_first_tile_id + 3;
   u8 tile_mid  = dialog_frame_first_tile_id + 4;
   u8 tile_e_r  = dialog_frame_first_tile_id + 5;
   u8 tile_c_bl = dialog_frame_first_tile_id + 6;
   u8 tile_e_b  = dialog_frame_first_tile_id + 7;
   u8 tile_c_br = dialog_frame_first_tile_id + 8;
   
   FillBgTilemapBufferRect(bg_layer, tile_c_tl, l,     t,      1,        1,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_e_t,  l + 1, t,      inner_w,  1,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_c_tr, r,     t,      1,        1,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_e_l,  l,     t + 1,  1,  inner_h,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_e_r,  r,     t + 1,  1,  inner_h,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_c_bl, l,     b,      1,        1,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_e_b,  l + 1, b,      inner_w,  1,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_c_br, r,     b,      1,        1,  palette_id);
}




void LuUI_SetupDarkenAllExceptRect(
   u8 bg_layer_to_darken
) {
   SetGpuReg(REG_OFFSET_WIN0H,  0);
   SetGpuReg(REG_OFFSET_WIN0V,  0);
   SetGpuReg(REG_OFFSET_WININ,  WINOUT_WIN01_BG_ALL);
   SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_ALL);
   SetGpuReg(REG_OFFSET_BLDCNT, (1 << bg_layer_to_darken) | BLDCNT_EFFECT_DARKEN);
   SetGpuReg(REG_OFFSET_BLDALPHA, 0);
   SetGpuReg(REG_OFFSET_BLDY, 4);
   SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
}
void LuUI_DoDarkenAllExceptRect(
   u8 x,
   u8 y,
   u8 width,
   u8 height
) {
   SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(x, x + width));
   SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(y, y + height));
}
void LuUI_StopDarkenAllExceptRect(
   u8 bg_layer_to_darken
) {
   SetGpuReg(REG_OFFSET_BLDCNT, 0);
}
   