#include "global.h"
#include "battle.h"
#include "title_screen.h"
#include "sprite.h"
#include "gba/m4a_internal.h"
#include "clear_save_data_menu.h"
#include "decompress.h"
#include "event_data.h"
#include "intro.h"
#include "m4a.h"
#include "main.h"
#include "main_menu.h"
#include "palette.h"
#include "reset_rtc_screen.h"
#include "berry_fix_program.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "scanline_effect.h"
#include "gpu_regs.h"
#include "trig.h"
#include "graphics.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#include "bg.h"
#include "lu/graphics_registers.h"
#include "lu/ui_helpers.h"
#include "lu/vram_layout_helpers.h"
#include "gba/isagbprint.h"

// Never instantiated. Just a marginally less hideous way to manage all this 
// compared to preprocessor macros. Unit of measurement is 4bpp tile IDs.
typedef struct {
   VRAMTile256Color pokemon_logo_tiles[256];
   
   VRAMTilemap pokemon_logo_tilemap;
   VRAMTilemap back_tilemap;
   VRAMTilemap front_tilemap;
   VRAMTilemap ashfall_tilemap;
   
   VRAMTile ALIGNED(BG_CHAR_SIZE) back_tiles[0xF0];
   VRAMTile front_tiles[0x229];
   VRAMTile ashfall;
} VRAMTileLayout;

_Static_assert(sizeof(VRAMTileLayout) <= BG_VRAM_SIZE, "VRAM layout is too large.");
_Static_assert(offsetof(VRAMTileLayout, pokemon_logo_tilemap) % BG_SCREEN_SIZE == 0, "Invalid offset of tilemap for logo layer.");
_Static_assert(offsetof(VRAMTileLayout, back_tilemap) % BG_SCREEN_SIZE == 0, "Invalid offset of tilemap for back layer.");
_Static_assert(offsetof(VRAMTileLayout, front_tilemap) % BG_SCREEN_SIZE == 0, "Invalid offset of tilemap for front layer.");

enum {
    TAG_VERSION = 1000,
    TAG_PRESS_START_COPYRIGHT,
    TAG_LOGO_SHINE,
};

#define VERSION_BANNER_RIGHT_TILEOFFSET 64
#define VERSION_BANNER_LEFT_X 98
#define VERSION_BANNER_RIGHT_X 162
#define VERSION_BANNER_Y 2
#define VERSION_BANNER_Y_GOAL 66
#define START_BANNER_X 128

#define CLEAR_SAVE_BUTTON_COMBO (B_BUTTON | SELECT_BUTTON | DPAD_UP)
#define RESET_RTC_BUTTON_COMBO (B_BUTTON | SELECT_BUTTON | DPAD_LEFT)
#define BERRY_UPDATE_BUTTON_COMBO (B_BUTTON | SELECT_BUTTON)
#define A_B_START_SELECT (A_BUTTON | B_BUTTON | START_BUTTON | SELECT_BUTTON)

static void MainCB2(void);
static void Task_TitleScreenPhase1(u8);
static void Task_TitleScreenPhase2(u8);
static void Task_TitleScreenPhase3(u8);
static void CB2_GoToMainMenu(void);
static void CB2_GoToClearSaveDataScreen(void);
static void CB2_GoToResetRtcScreen(void);
static void CB2_GoToBerryFixScreen(void);
static void CB2_GoToCopyrightScreen(void);
static void UpdateLegendaryMarkingColor(u8);

static void SpriteCB_VersionBannerLeft(struct Sprite *sprite);
static void SpriteCB_VersionBannerRight(struct Sprite *sprite);
static void SpriteCB_PressStartCopyrightBanner(struct Sprite *sprite);
static void SpriteCB_PokemonLogoShine(struct Sprite *sprite);

static const u32 sTitleScreenLogoShineGfx[] = INCBIN_U32("graphics/title_screen/logo_shine.4bpp.lz");

static const u32 sTitleScreenAshfallBgGfx[] = INCBIN_U32("graphics/lu/title_screen/ash_bg_layer_tiles.4bpp.lz");static const u32 sTitleScreenAshfallBgTilemap[] = INCBIN_U32("graphics/lu/title_screen/ash_bg_layer_tiles.bin.lz");

static const u32 sTitleScreenArtBgFrontGfx[] = INCBIN_U32("graphics/lu/title_screen/art_front_tiles.4bpp.lz");
static const u32 sTitleScreenArtBgFrontTilemap[] = INCBIN_U32("graphics/lu/title_screen/art_front_tiles.bin.lz");

static const u32 sTitleScreenArtBgBackGfx[] = INCBIN_U32("graphics/lu/title_screen/art_back_tiles.4bpp.lz");
static const u32 sTitleScreenArtBgBackTilemap[] = INCBIN_U32("graphics/lu/title_screen/art_back_tiles.bin.lz");

static const u16 sPokemonLogoPalette[] = INCBIN_U16("graphics/title_screen/pokemon_logo.gbapal");
static const u16 sTitleScreenBgArtPalettes[] = INCBIN_U16(
   "graphics/lu/title_screen/art_back_tiles.gbapal",
   "graphics/lu/title_screen/art_front_tiles.gbapal"
);

static const struct BgTemplate sBgTemplates[] = {
   {  // Pokemon logo
      .bg = 2, // only BG2 can be a bitmap
      .charBaseIndex = 0,
      .mapBaseIndex  = VRAM_BG_MapBaseIndex(VRAMTileLayout, pokemon_logo_tilemap),
      .screenSize    = 1, // Affine 256x256px
      .paletteMode   = 1, // 256-color
      .priority      = 0,
      .baseTile      = 0,
   },
   {  // Back-layer art (treeline, mountains, and sky)
      .bg = 1,
      .charBaseIndex = VRAM_BG_CharBaseIndex(VRAMTileLayout, back_tiles),
      .mapBaseIndex  = VRAM_BG_MapBaseIndex(VRAMTileLayout, back_tilemap),
      .screenSize    = 0, // Text 256x256px
      .paletteMode   = 0, // 16-color
      .priority      = 3,
      .baseTile      = 0, // baseTile only works with LoadBgTiles, not for loading compressed tile(map)s
   },
   {  // Front-layer art (Spindas and grass)
      .bg = 0,
      .charBaseIndex = VRAM_BG_CharBaseIndex(VRAMTileLayout, back_tiles),
      .mapBaseIndex  = VRAM_BG_MapBaseIndex(VRAMTileLayout, front_tilemap),
      .screenSize    = 0, // Text 256x256px
      .paletteMode   = 0, // 16-color
      .priority      = 2,
      .baseTile      = 0, // baseTile only works with LoadBgTiles, not for loading compressed tile(map)s
   },
   //
   // TODO: If BG layer 2 is an affine layer (which it is), then BG layer 3 can only 
   //       be an affine layer or disabled. Affine is fine for our purposes -- we can 
   //       look into adjusting its translation and scale that way -- but for now, we 
   //       have BG3 disabled. Research `DISPCNT_MODE_1` for info on how to control 
   //       which layers are affine, and Ctrl + F for where it's used below.
   //
   {  // Ashfall BG layer
      .bg = 3,
      .charBaseIndex = VRAM_BG_CharBaseIndex(VRAMTileLayout, ashfall),
      .mapBaseIndex  = VRAM_BG_MapBaseIndex(VRAMTileLayout, ashfall_tilemap),
      .screenSize    = 0, // Text 256x256px
      .paletteMode   = 0, // 16-color
      .priority      = 1,
      .baseTile      = 0, // baseTile only works with LoadBgTiles, not for loading compressed tile(map)s
   },
};

// Used to blend "Emerald Version" as it passes over over the Pokémon banner.
// Also used by the intro to blend the Game Freak name/logo in and out as they appear and disappear
const u16 gTitleScreenAlphaBlend[64] =
{
    BLDALPHA_BLEND(16, 0),
    BLDALPHA_BLEND(16, 1),
    BLDALPHA_BLEND(16, 2),
    BLDALPHA_BLEND(16, 3),
    BLDALPHA_BLEND(16, 4),
    BLDALPHA_BLEND(16, 5),
    BLDALPHA_BLEND(16, 6),
    BLDALPHA_BLEND(16, 7),
    BLDALPHA_BLEND(16, 8),
    BLDALPHA_BLEND(16, 9),
    BLDALPHA_BLEND(16, 10),
    BLDALPHA_BLEND(16, 11),
    BLDALPHA_BLEND(16, 12),
    BLDALPHA_BLEND(16, 13),
    BLDALPHA_BLEND(16, 14),
    BLDALPHA_BLEND(16, 15),
    BLDALPHA_BLEND(15, 16),
    BLDALPHA_BLEND(14, 16),
    BLDALPHA_BLEND(13, 16),
    BLDALPHA_BLEND(12, 16),
    BLDALPHA_BLEND(11, 16),
    BLDALPHA_BLEND(10, 16),
    BLDALPHA_BLEND(9, 16),
    BLDALPHA_BLEND(8, 16),
    BLDALPHA_BLEND(7, 16),
    BLDALPHA_BLEND(6, 16),
    BLDALPHA_BLEND(5, 16),
    BLDALPHA_BLEND(4, 16),
    BLDALPHA_BLEND(3, 16),
    BLDALPHA_BLEND(2, 16),
    BLDALPHA_BLEND(1, 16),
    BLDALPHA_BLEND(0, 16),
    [32 ... 63] = BLDALPHA_BLEND(0, 16)
};

static const struct OamData sVersionBannerLeftOamData =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_8BPP,
    .shape = SPRITE_SHAPE(64x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x32),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct OamData sVersionBannerRightOamData =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_8BPP,
    .shape = SPRITE_SHAPE(64x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x32),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sVersionBannerLeftAnimSequence[] =
{
    ANIMCMD_FRAME(0, 30),
    ANIMCMD_END,
};

static const union AnimCmd sVersionBannerRightAnimSequence[] =
{
    ANIMCMD_FRAME(VERSION_BANNER_RIGHT_TILEOFFSET, 30),
    ANIMCMD_END,
};

static const union AnimCmd *const sVersionBannerLeftAnimTable[] =
{
    sVersionBannerLeftAnimSequence,
};

static const union AnimCmd *const sVersionBannerRightAnimTable[] =
{
    sVersionBannerRightAnimSequence,
};

static const struct SpriteTemplate sVersionBannerLeftSpriteTemplate =
{
    .tileTag = TAG_VERSION,
    .paletteTag = TAG_VERSION,
    .oam = &sVersionBannerLeftOamData,
    .anims = sVersionBannerLeftAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_VersionBannerLeft,
};

static const struct SpriteTemplate sVersionBannerRightSpriteTemplate =
{
    .tileTag = TAG_VERSION,
    .paletteTag = TAG_VERSION,
    .oam = &sVersionBannerRightOamData,
    .anims = sVersionBannerRightAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_VersionBannerRight,
};

static const struct CompressedSpriteSheet sSpriteSheet_EmeraldVersion[] =
{
    {
        .data = gTitleScreenEmeraldVersionGfx,
        .size = 0x1000,
        .tag = TAG_VERSION
    },
    {},
};

static const struct OamData sOamData_CopyrightBanner =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sAnim_PressStart_0[] =
{
    ANIMCMD_FRAME(1, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_PressStart_1[] =
{
    ANIMCMD_FRAME(5, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_PressStart_2[] =
{
    ANIMCMD_FRAME(9, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_PressStart_3[] =
{
    ANIMCMD_FRAME(13, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_PressStart_4[] =
{
    ANIMCMD_FRAME(17, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_Copyright_0[] =
{
    ANIMCMD_FRAME(21, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_Copyright_1[] =
{
    ANIMCMD_FRAME(25, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_Copyright_2[] =
{
    ANIMCMD_FRAME(29, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_Copyright_3[] =
{
    ANIMCMD_FRAME(33, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_Copyright_4[] =
{
    ANIMCMD_FRAME(37, 4),
    ANIMCMD_END,
};

// The "Press Start" and copyright graphics are each 5 32x8 segments long
#define NUM_PRESS_START_FRAMES 5
#define NUM_COPYRIGHT_FRAMES 5

static const union AnimCmd *const sStartCopyrightBannerAnimTable[NUM_PRESS_START_FRAMES + NUM_COPYRIGHT_FRAMES] = {
   sAnim_PressStart_0,
   sAnim_PressStart_1,
   sAnim_PressStart_2,
   sAnim_PressStart_3,
   sAnim_PressStart_4,
   [NUM_PRESS_START_FRAMES] =
   sAnim_Copyright_0,
   sAnim_Copyright_1,
   sAnim_Copyright_2,
   sAnim_Copyright_3,
   sAnim_Copyright_4,
};

static const struct SpriteTemplate sStartCopyrightBannerSpriteTemplate = {
   .tileTag = TAG_PRESS_START_COPYRIGHT,
   .paletteTag = TAG_PRESS_START_COPYRIGHT,
   .oam = &sOamData_CopyrightBanner,
   .anims = sStartCopyrightBannerAnimTable,
   .images = NULL,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback = SpriteCB_PressStartCopyrightBanner,
};

static const struct CompressedSpriteSheet sSpriteSheet_PressStart[] = {
   {
      .data = gTitleScreenPressStartGfx,
      .size = 0x520,
      .tag = TAG_PRESS_START_COPYRIGHT
   },
   {},
};

static const struct SpritePalette sSpritePalette_PressStart[] = {
   {
      .data = gTitleScreenPressStartPal,
      .tag = TAG_PRESS_START_COPYRIGHT
   },
   {},
};

static const struct OamData sPokemonLogoShineOamData = {
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sPokemonLogoShineAnimSequence[] = {
   ANIMCMD_FRAME(0, 4),
   ANIMCMD_END,
};

static const union AnimCmd *const sPokemonLogoShineAnimTable[] =
{
    sPokemonLogoShineAnimSequence,
};

static const struct SpriteTemplate sPokemonLogoShineSpriteTemplate =
{
    .tileTag = TAG_LOGO_SHINE,
    .paletteTag = TAG_PRESS_START_COPYRIGHT,
    .oam = &sPokemonLogoShineOamData,
    .anims = sPokemonLogoShineAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_PokemonLogoShine,
};

static const struct CompressedSpriteSheet sPokemonLogoShineSpriteSheet[] =
{
    {
        .data = sTitleScreenLogoShineGfx,
        .size = 0x800,
        .tag = TAG_LOGO_SHINE
    },
    {},
};

// Task data for the main title screen tasks (Task_TitleScreenPhase#)
#define tCounter    data[0]
#define tSkipToNext data[1]
#define tPointless  data[2] // Incremented but never used to do anything.
#define tBg2Y       data[3]
#define tBg1Y       data[4]

// Sprite data for sVersionBannerLeftSpriteTemplate / sVersionBannerRightSpriteTemplate
#define sAlphaBlendIdx data[0]
#define sParentTaskId  data[1]
static void SpriteCB_VersionBannerLeft(struct Sprite *sprite) {
   if (gTasks[sprite->sParentTaskId].tSkipToNext) {
      sprite->oam.objMode = ST_OAM_OBJ_NORMAL;
      sprite->y = VERSION_BANNER_Y_GOAL;
   } else {
      if (sprite->y != VERSION_BANNER_Y_GOAL)
         sprite->y++;
      if (sprite->sAlphaBlendIdx != 0)
         sprite->sAlphaBlendIdx--;
      SetGpuReg(REG_OFFSET_BLDALPHA, gTitleScreenAlphaBlend[sprite->sAlphaBlendIdx]);
   }
}
static void SpriteCB_VersionBannerRight(struct Sprite *sprite) {
   if (gTasks[sprite->sParentTaskId].tSkipToNext) {
      sprite->oam.objMode = ST_OAM_OBJ_NORMAL;
      sprite->y = VERSION_BANNER_Y_GOAL;
   } else {
      if (sprite->y != VERSION_BANNER_Y_GOAL)
         sprite->y++;
   }
}


// Sprite data for SpriteCB_PressStartCopyrightBanner
#define sAnimate data[0]
#define sTimer   data[1]
static void SpriteCB_PressStartCopyrightBanner(struct Sprite *sprite) {
   if (sprite->sAnimate == TRUE) {
      // Alternate between hidden and shown every 32th frame
      if (++sprite->sTimer & 32)
         sprite->invisible = FALSE;
      else
         sprite->invisible = TRUE;
   } else {
      sprite->invisible = FALSE;
   }
}

static void CreatePressStartBanner(s16 x, s16 y) {
   x -= 64;
   for (u8 i = 0; i < NUM_PRESS_START_FRAMES; i++, x += 32) {
      u8 spriteId = CreateSprite(&sStartCopyrightBannerSpriteTemplate, x, y, 0);
      StartSpriteAnim(&gSprites[spriteId], i);
      gSprites[spriteId].sAnimate = TRUE;
   }
}
static void CreateCopyrightBanner(s16 x, s16 y) {
   x -= 64;
   for (u8 i = 0; i < NUM_COPYRIGHT_FRAMES; i++, x += 32) {
      u8 spriteId = CreateSprite(&sStartCopyrightBannerSpriteTemplate, x, y, 0);
      StartSpriteAnim(&gSprites[spriteId], i + NUM_PRESS_START_FRAMES);
   }
}

#undef sAnimate
#undef sTimer

// Defines for SpriteCB_PokemonLogoShine
enum {
    SHINE_MODE_SINGLE_NO_BG_COLOR,
    SHINE_MODE_DOUBLE,
    SHINE_MODE_SINGLE,
};

#define SHINE_SPEED  4
#define SHINE_BG_CHANGE_PER_TICK 2

#define sMode     data[0]
#define sBgColor  data[1]

static void SpriteCB_PokemonLogoShine(struct Sprite *sprite) {
   if (sprite->x < DISPLAY_WIDTH + 32) {
      // In any mode except SHINE_MODE_SINGLE_NO_BG_COLOR the background
      // color will change, in addition to the shine sprite moving.
      if (sprite->sMode != SHINE_MODE_SINGLE_NO_BG_COLOR) {
         if (sprite->x < DISPLAY_WIDTH / 2) {
            // Brighten background color
            if (sprite->sBgColor < 31 - SHINE_BG_CHANGE_PER_TICK)
               sprite->sBgColor += SHINE_BG_CHANGE_PER_TICK;
            else
               sprite->sBgColor = 31;
         } else {
            // Darken background color
            if (sprite->sBgColor > SHINE_BG_CHANGE_PER_TICK)
               sprite->sBgColor -= SHINE_BG_CHANGE_PER_TICK;
            else
               sprite->sBgColor = 0;
         }

         u16 backgroundColor = _RGB(sprite->sBgColor, sprite->sBgColor, sprite->sBgColor);

         // Flash the background green for 4 frames of movement.
         // Otherwise use the updating color.
         if (sprite->x == DISPLAY_WIDTH / 2 + (3 * SHINE_SPEED)
          || sprite->x == DISPLAY_WIDTH / 2 + (4 * SHINE_SPEED)
          || sprite->x == DISPLAY_WIDTH / 2 + (5 * SHINE_SPEED)
          || sprite->x == DISPLAY_WIDTH / 2 + (6 * SHINE_SPEED)
         )
            gPlttBufferFaded[0] = RGB(24, 31, 12);
         else
            gPlttBufferFaded[0] = backgroundColor;
      }

      sprite->x += SHINE_SPEED;
   } else {
      // Sprite has moved fully offscreen
      gPlttBufferFaded[0] = RGB_WHITE;
      DestroySprite(sprite);
      DebugPrintf("[Title Screen] [Pokemon Logo Shine, Normal] Done.");
   }
}

static void SpriteCB_PokemonLogoShine_Fast(struct Sprite *sprite) {
   if (sprite->x < DISPLAY_WIDTH + 32) {
      sprite->x += SHINE_SPEED * 2;
   } else {
      DestroySprite(sprite);
      DebugPrintf("[Title Screen] [Pokemon Logo Shine, Fast] Done.");
   }
}

static void StartPokemonLogoShine(u8 mode) {
    u8 spriteId;

   switch (mode) {
    case SHINE_MODE_SINGLE_NO_BG_COLOR:
      case SHINE_MODE_SINGLE:
         // Create one regular shine sprite.
         // If mode is SHINE_MODE_SINGLE it will also change the background color.
         spriteId = CreateSprite(&sPokemonLogoShineSpriteTemplate, 0, 68, 0);
         gSprites[spriteId].oam.objMode = ST_OAM_OBJ_WINDOW;
         gSprites[spriteId].sMode = mode;
         break;
      case SHINE_MODE_DOUBLE:
         // Create an invisible sprite with mode set to update the background color
         spriteId = CreateSprite(&sPokemonLogoShineSpriteTemplate, 0, 68, 0);
         gSprites[spriteId].oam.objMode = ST_OAM_OBJ_WINDOW;
         gSprites[spriteId].sMode = mode;
         gSprites[spriteId].invisible = TRUE;

         // Create two faster shine sprites
         spriteId = CreateSprite(&sPokemonLogoShineSpriteTemplate, 0, 68, 0);
         gSprites[spriteId].callback = SpriteCB_PokemonLogoShine_Fast;
         gSprites[spriteId].oam.objMode = ST_OAM_OBJ_WINDOW;

         spriteId = CreateSprite(&sPokemonLogoShineSpriteTemplate, -80, 68, 0);
         gSprites[spriteId].callback = SpriteCB_PokemonLogoShine_Fast;
         gSprites[spriteId].oam.objMode = ST_OAM_OBJ_WINDOW;
         break;
   }
}

#undef sMode
#undef sBgColor

static void VBlankCB(void) {
   ScanlineEffect_InitHBlankDmaTransfer();
   LoadOam();
   ProcessSpriteCopyRequests();
   TransferPlttBuffer();
   SetGpuReg(REG_OFFSET_BG1VOFS, gBattle_BG1_Y);
}

static const struct ColorEffectParams sInitialColorEffect = {
   .target1Layers  = COLOR_EFFECT_LAYER_BG2,
   .effect         = COLOR_EFFECT_BRIGHTNESS_INCREASE,
   .target2Layers  = COLOR_EFFECT_LAYER_NONE,
   .alpha          = { 0 },
   .brightness     = {
      .coefficient = 12,
   },
};
static const struct ScreenWindowParams sInitialScreenWindows = {
   .bounds = { 0 },
   .mask = {
      .win_0 = {
         .layers = SCREEN_WINDOW_MASK_LAYER_BG_ALL | SCREEN_WINDOW_MASK_LAYER_OBJ,
         .enable_color_effect = FALSE,
      },
      .win_outside = {
         .layers = SCREEN_WINDOW_MASK_LAYER_BG_ALL | SCREEN_WINDOW_MASK_LAYER_OBJ,
         .enable_color_effect = FALSE,
      },
   },
};

// Alpha-blend the "EMERALD VERSION" sprite to fade it in.
static const struct ColorEffectParams sPhase2ColorEffect = {
   .target1Layers  = COLOR_EFFECT_LAYER_OBJ,
   .effect         = COLOR_EFFECT_ALPHA_BLEND,
   .target2Layers  = COLOR_EFFECT_LAYER_ALL,
   .alpha          = { // the alpha params here will be overwritten by sVersionBannerLeftSpriteTemplate
      .target1Coefficient = 16,
      .target2Coefficient =  0,
   },
   .brightness = { 0 },
};

void CB2_InitTitleScreen(void) {
   switch (gMain.state) {
      default:
      case 0:
         SetVBlankCallback(NULL);
         ResetBlendRegisters();
         LuUI_ResetBackgroundsAndVRAM();
         ResetPaletteFade();
         gPlttBufferFaded[0] = RGB_WHITE;
         gMain.state = 1;
         break;
      case 1:
         VRAM_LoadCompressedBGData(gTitleScreenPokemonLogoGfx,     pokemon_logo_tiles);
         VRAM_LoadCompressedBGData(gTitleScreenPokemonLogoTilemap, pokemon_logo_tilemap);
         VRAM_LoadBGPaletteData(sPokemonLogoPalette, 0);
        
         VRAM_LoadBGPaletteData(sTitleScreenBgArtPalettes, 14);
         VRAM_LoadCompressedBGData(sTitleScreenArtBgBackGfx,      back_tiles);
         VRAM_LoadCompressedBGData(sTitleScreenArtBgBackTilemap,  back_tilemap);
         VRAM_LoadCompressedBGData(sTitleScreenArtBgFrontGfx,     front_tiles);
         VRAM_LoadCompressedBGData(sTitleScreenArtBgFrontTilemap, front_tilemap);
         VRAM_LoadCompressedBGData(sTitleScreenAshfallBgGfx,      ashfall);
         VRAM_LoadCompressedBGData(sTitleScreenAshfallBgTilemap,  ashfall_tilemap);
        
         ScanlineEffect_Stop();
         ResetTasks();
         ResetSpriteData();
         FreeAllSpritePalettes();
         gReservedSpritePaletteCount = 9;
         LoadCompressedSpriteSheet(&sSpriteSheet_EmeraldVersion[0]);
         LoadCompressedSpriteSheet(&sSpriteSheet_PressStart[0]);
         LoadCompressedSpriteSheet(&sPokemonLogoShineSpriteSheet[0]);
         LoadPalette(gTitleScreenEmeraldVersionPal, OBJ_PLTT_ID(0), PLTT_SIZE_4BPP);
         LoadSpritePalette(&sSpritePalette_PressStart[0]);
         gMain.state = 2;
         break;
      case 2:
         {
            u8 taskId = CreateTask(Task_TitleScreenPhase1, 0);

            gTasks[taskId].tCounter = 256;
            gTasks[taskId].tSkipToNext = FALSE;
            gTasks[taskId].tPointless = -16;
            gTasks[taskId].tBg2Y = -32;
            gMain.state = 3;
         }
         break;
      case 3:
         // Fade in the background artwork.
         BeginNormalPaletteFade(PALETTES_ALL, 1, 16, 0, RGB_WHITEALPHA);
         SetVBlankCallback(VBlankCB);
         gMain.state = 4;
         break;
      case 4:
         PanFadeAndZoomScreen(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, 0x100, 0);
         SetScreenWindowParams(&sInitialScreenWindows);
         SetBlendRegisters(&sInitialColorEffect);
         InitBgsFromTemplates(DISPCNT_MODE_1, sBgTemplates, ARRAY_COUNT(sBgTemplates));
         ShowBg(2);
         SetBgAffine( // Pokemon logo
            2, // only BG2 can be a bitmap
            -29 * 256, // srcCenterX
            -32 * 256, // srcCenterY
            0, // dispCenterX
            0, // dispCenterY
            256, // scaleX (256 = 100%)
            256, // scaleY (256 = 100%)
            0  // rotationAngle
         );
         EnableInterrupts(INTR_FLAG_VBLANK);
         SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
         SetScreenWindowsEnabled(SCREEN_WINDOW_0 | SCREEN_WINDOW_OBJ, TRUE);
         m4aSongNumStart(MUS_TITLE);
         gMain.state = 5;
         break;
      case 5:
         if (!UpdatePaletteFade()) {
            StartPokemonLogoShine(SHINE_MODE_SINGLE_NO_BG_COLOR);
            //ScanlineEffect_InitWave(0, DISPLAY_HEIGHT, 4, 4, 0, SCANLINE_EFFECT_REG_BG1HOFS, TRUE);
            SetMainCallback2(MainCB2);
         }
         break;
   }
}

static void MainCB2(void) {
   RunTasks();
   AnimateSprites();
   BuildOamBuffer();
   UpdatePaletteFade();
}

// Shine the Pokémon logo two more times, and fade in the version banner
static void Task_TitleScreenPhase1(u8 taskId) {
   // Skip to next phase when A, B, Start, or Select is pressed
   if (JOY_NEW(A_B_START_SELECT) || gTasks[taskId].tSkipToNext) {
      gTasks[taskId].tSkipToNext = TRUE;
      gTasks[taskId].tCounter = 0;
   }

   if (gTasks[taskId].tCounter != 0) {
      u16 frameNum = gTasks[taskId].tCounter;
      if (frameNum == 176)
         StartPokemonLogoShine(SHINE_MODE_DOUBLE);
      else if (frameNum == 64)
         StartPokemonLogoShine(SHINE_MODE_SINGLE);

      gTasks[taskId].tCounter--;
   } else {
      u8 spriteId;

      SetScreenWindowsEnabled(0, FALSE);
      ResetScreenWindows();
      SetBlendRegisters(&sPhase2ColorEffect);

        // Create left side of version banner
      spriteId = CreateSprite(&sVersionBannerLeftSpriteTemplate, VERSION_BANNER_LEFT_X, VERSION_BANNER_Y, 0);
      gSprites[spriteId].sAlphaBlendIdx = ARRAY_COUNT(gTitleScreenAlphaBlend);
      gSprites[spriteId].sParentTaskId = taskId;

        // Create right side of version banner
      spriteId = CreateSprite(&sVersionBannerRightSpriteTemplate, VERSION_BANNER_RIGHT_X, VERSION_BANNER_Y, 0);
      gSprites[spriteId].sParentTaskId = taskId;

      gTasks[taskId].tCounter = 144;
      gTasks[taskId].func = Task_TitleScreenPhase2;
      DebugPrintf("[Title Screen] Advancing to phase 2.");
   }
}

#undef sParentTaskId
#undef sAlphaBlendIdx

// Create "Press Start" and copyright banners, and slide Pokémon logo up
static void Task_TitleScreenPhase2(u8 taskId) {
   // Skip to next phase when A, B, Start, or Select is pressed
   if (JOY_NEW(A_B_START_SELECT) || gTasks[taskId].tSkipToNext) {
      gTasks[taskId].tSkipToNext = TRUE;
      gTasks[taskId].tCounter = 0;
   }

   if (gTasks[taskId].tCounter != 0) {
      gTasks[taskId].tCounter--;
   } else {
      gTasks[taskId].tSkipToNext = TRUE;
      ResetBlendRegisters();
      ShowBg(0);
      ShowBg(1);
      ShowBg(2);
      CreatePressStartBanner(START_BANNER_X, 108);
      CreateCopyrightBanner(START_BANNER_X, 148);
      gTasks[taskId].tBg1Y = 0;
      gTasks[taskId].func = Task_TitleScreenPhase3;
      UnfadePlttBuffer( (1 << 14) | (1 << 15) );
      BeginNormalPaletteFade(
         (1 << 14) | (1 << 15),
         -1,
         16,
         0,
         RGB_WHITE
      );
      DebugPrintf("[Title Screen] Advancing to phase 3.");
   }

   if (!(gTasks[taskId].tCounter & 3) && gTasks[taskId].tPointless != 0)
      gTasks[taskId].tPointless++;
   if (!(gTasks[taskId].tCounter & 1) && gTasks[taskId].tBg2Y != 0)
      gTasks[taskId].tBg2Y++;

   // Slide Pokémon logo up
   u32 yPos = gTasks[taskId].tBg2Y * 256;
   SetGpuReg(REG_OFFSET_BG2Y_L, yPos);
   SetGpuReg(REG_OFFSET_BG2Y_H, yPos / 0x10000);

   gTasks[taskId].data[5] = 15; // Unused
   gTasks[taskId].data[6] = 6;  // Unused
}

// Show background art and process main title screen input
static void Task_TitleScreenPhase3(u8 taskId) {
   if (JOY_NEW(A_BUTTON) || JOY_NEW(START_BUTTON)) {
      FadeOutBGM(4);
      BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_WHITEALPHA);
      SetMainCallback2(CB2_GoToMainMenu);
   } else if (JOY_HELD(CLEAR_SAVE_BUTTON_COMBO) == CLEAR_SAVE_BUTTON_COMBO) {
      SetMainCallback2(CB2_GoToClearSaveDataScreen);
   } else if (
      JOY_HELD(RESET_RTC_BUTTON_COMBO) == RESET_RTC_BUTTON_COMBO
      && CanResetRTC() == TRUE
   ) {
      FadeOutBGM(4);
      BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
      SetMainCallback2(CB2_GoToResetRtcScreen);
   } else if (JOY_HELD(BERRY_UPDATE_BUTTON_COMBO) == BERRY_UPDATE_BUTTON_COMBO) {
      FadeOutBGM(4);
      BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
      SetMainCallback2(CB2_GoToBerryFixScreen);
   } else {
      SetGpuReg(REG_OFFSET_BG2Y_L, 0);
      SetGpuReg(REG_OFFSET_BG2Y_H, 0);
      if (++gTasks[taskId].tCounter & 1) {
         gTasks[taskId].tBg1Y++;
         //gBattle_BG1_Y = gTasks[taskId].tBg1Y / 2;
         gBattle_BG1_Y = 0;
         gBattle_BG1_X = 0;
      }
      if ((gMPlayInfo_BGM.status & 0xFFFF) == 0) {
         BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_WHITEALPHA);
         SetMainCallback2(CB2_GoToCopyrightScreen);
      }
   }
}

static void CB2_GoToMainMenu(void) {
   if (!UpdatePaletteFade())
      SetMainCallback2(CB2_InitMainMenu);
}

static void CB2_GoToCopyrightScreen(void) {
    if (!UpdatePaletteFade())
      SetMainCallback2(CB2_InitCopyrightScreenAfterTitleScreen);
}

static void CB2_GoToClearSaveDataScreen(void) {
   if (!UpdatePaletteFade())
      SetMainCallback2(CB2_InitClearSaveDataScreen);
}

static void CB2_GoToResetRtcScreen(void) {
   if (!UpdatePaletteFade())
      SetMainCallback2(CB2_InitResetRtcScreen);
}

static void CB2_GoToBerryFixScreen(void) {
   if (!UpdatePaletteFade()) {
      m4aMPlayAllStop();
      SetMainCallback2(CB2_InitBerryFixProgram);
   }
}
