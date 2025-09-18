#include "lu/gfx_test_screen.h"
#include "lu/c.h"
#include "lu/gfxutils.h"
#include "lu/ui_helpers.h"
#include "lu/vram_layout_helpers_new.h"
#include "gba/gba.h"
#include "main.h"
#include "task.h"

#include "constants/rgb.h"
#include "gba/isagbprint.h"
#include "bg.h"
#include "gpu_regs.h"
#include "io_reg.h"
#include "malloc.h"
#include "palette.h"
#include "sprite.h"
#include "text.h"
#include "window.h"

#include "hicolor/hicolor.h"

struct MenuState {
   void(*callback)(void);
   u8    task_id;
   bool8 initialized;
   u8    timer;
   u8    counter;
   
   u8 tile_src_buffer[TILE_SIZE_4BPP * (128 / TILE_WIDTH) * (32 / TILE_HEIGHT)];
   
   u8 sprite_ids[32];
};
static EWRAM_DATA struct MenuState* sMenuState = NULL;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Task_FrameHandler(u8);
static void CB2_Init(void);
static void CB2_Idle(void);
static void VBlankCB(void);
static void VBlankCB_BGPalOnly(void);
static void TestSpecificFrameHandler(void);
static void TestShutdownHandler(void);

extern void LuGfxTestScreen(void(*callback)(void)) {
   ResetTasks();
   SetVBlankCallback(NULL);
   SetHBlankCallback(NULL);
   LuUI_ResetBackgroundsAndVRAM();
   LuUI_ResetSpritesAndEffects();
   FreeAllWindowBuffers();
   DeactivateAllTextPrinters();
   
   DebugPrintf("GFX Test Screen: Initializing...");
   {
      AGB_ASSERT(sMenuState == NULL);
      sMenuState = (struct MenuState*) AllocZeroed(sizeof(struct MenuState));
      sMenuState->callback = callback;
   }
   
   SetMainCallback2(CB2_Init);
   gMain.state = 0;
}

// ---------------------------------------------------------

static void Task_FrameHandler(u8 task_id) {
   ++sMenuState->timer;
   TestSpecificFrameHandler();
   if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON)) {
      DebugPrintf("GFX Test Screen: Exiting...");
      DestroyTask(task_id);
      DebugPrintf("GFX Test Screen: Running shutdown handler...");
      TestShutdownHandler();
      DebugPrintf("GFX Test Screen: Freeing state and tilemaps...");
      auto callback = sMenuState->callback;
      Free(sMenuState);
      sMenuState = NULL;
      for(u8 i = 0; i < 4; ++i) {
         auto tilemap = GetBgTilemapBuffer(i);
         if (tilemap) {
            SetBgTilemapBuffer(i, NULL);
            Free(tilemap);
         }
      }
      DebugPrintf("GFX Test Screen: Exited.");
      (callback)();
   }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static const u8 sKeypadIconTiles[] = INCBIN_U8("graphics/fonts/keypad_icons.4bpp");

vram_bg_layout {
   vram_bg_tilemap tilemaps[4];
   
   vram_bg_tile blank_tile;
   vram_bg_tile dpad_tiles[sizeof(sKeypadIconTiles) / TILE_SIZE_4BPP];
   vram_bg_tile recolored_tiles[sizeof(sKeypadIconTiles) / TILE_SIZE_4BPP];
};
__verify_vram_bg_layout;

enum {
   SPRITE_GFX_TAG = 0x1234,
};
static const u8 sSprite[] = INCBIN_U8("graphics/lu/test-circle-16color.4bpp");
static const struct SpriteSheet sSpriteSheets[] = {
   {sSprite, sizeof(sSprite), SPRITE_GFX_TAG},
   {}
};
static const struct SpriteFrameImage sSpriteFrameImage = {
   .data = &sSprite,
   .size = sizeof(sSprite)
};
static const struct OamData sSpriteOAM = {
   .affineMode = 0,
   .objMode    = ST_OAM_OBJ_NORMAL,
   .shape      = SPRITE_SHAPE(16x16),
   .size       = SPRITE_SIZE(16x16),
};
static const struct SpriteTemplate sSpriteTemplate = {
   .tileTag     = SPRITE_GFX_TAG,
   .paletteTag  = TAG_NONE,
   .oam         = &sSpriteOAM,
   .anims       = gDummySpriteAnimTable,
   .images      = NULL,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback    = SpriteCallbackDummy,
};

static const u16 sSpritePalettes[32][16];

static const struct BgTemplate sBgTemplates[] = {
   {
      .bg            = 0,
      .charBaseIndex = 0,
      .mapBaseIndex  = 0,
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 3,
      .baseTile      = 0
   },
};

static const u16 sTestPalette[] = {
   RGB(31, 31, 31),
   RGB(31,  0,  0),
   RGB(31, 15,  0),
   RGB(31, 31,  0),
   RGB(15, 28,  0),
   RGB( 0, 24,  0),
   RGB( 0, 31, 31),
   RGB( 0, 15, 31),
   RGB( 0,  0, 31),
   RGB(12,  4, 28),
   RGB(28,  6, 28),
   RGB(24, 24, 24),
   RGB(20, 20, 20),
   RGB(16, 16, 16),
   RGB(12, 12, 12),
   RGB( 7,  7,  7),
};

static void CB2_Init(void) {
   switch (gMain.state) {
      case 0:
         SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
         EnableInterrupts(INTR_FLAG_HBLANK | INTR_FLAG_VBLANK);
         gMain.state++;
         break;
         
      //
      // Code here is specific to each test, and should set up the appropriate 
      // graphics resources.
      //
      case 1:
         HiColor_Reset();
         HiColor_SetAvailableVRAMPalettes(0xFFFC);
         InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
         for(u8 i = 0; i < ARRAY_COUNT(sBgTemplates); ++i) {
            ShowBg(i);
            u16 size = GetBgAttribute(i, BG_ATTR_METRIC);
            if (size != 0xFFFF) {
               SetBgTilemapBuffer(i, AllocZeroed(size));
               DebugPrintf("Allocated tilemap buffer for BG layer %u.", i);
            } else {
               SetBgTilemapBuffer(i, NULL);
               DebugPrintf("FAILED to allocate tilemap buffer for BG layer %u!", i);
            }
            FillBgTilemapBufferRect(0, V_TILE_ID(blank_tile), 0, 0, 32, 32, 0);
         }
         for(u8 i = ARRAY_COUNT(sBgTemplates); i < 4; ++i) {
            SetBgTilemapBuffer(i, NULL);
            DebugPrintf("Cleared tilemap buffer for unused BG layer %u.", i);
         }
         LoadPalette(sTestPalette, 0, sizeof(sTestPalette));
         gMain.state++;
         break;
      case 2:
         LoadSpriteSheets(sSpriteSheets);
         {
            const int SPRITES_PER_ROW = 7;
            for(u8 i = 0; i < 32; ++i) {
               u8 sprite_id = sMenuState->sprite_ids[i] = CreateSprite(
                  &sSpriteTemplate,
                  i % SPRITES_PER_ROW * 32 + 12,
                  i / SPRITES_PER_ROW * 32 + 12,
                  0
               );
               if (sprite_id != SPRITE_NONE) {
                  HiColor_RegisterSprite(&gSprites[sprite_id], 0x1000 + i, FALSE);
                  gSprites[sprite_id].invisible = FALSE;
               } else {
                  DebugPrintf("FAILED to create sprite %u.", i);
               }
            }
         }
         DebugPrintf("Sprites created.");
         gMain.state++;
         break;
      case 3:
         for(u8 i = 0; i < 32; ++i) {
            HiColor_OverwritePaletteByTag_4ByteAlignedSrc(0x1000 + i, sSpritePalettes[i]);
         }
         DebugPrintf("HiColor palettes loaded.");
         gMain.state++;
         break;
         
      default:
         SetVBlankCallback(VBlankCB);
         SetVBlankCallback(VBlankCB_BGPalOnly);
         SetHBlankCallback(HiColor_HBlank);
         SetMainCallback2(CB2_Idle);
         sMenuState->task_id = CreateTask(Task_FrameHandler, 50);
         DebugPrintf("GFX Test Screen: Showing...");
         return;
   }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void CB2_Idle(void) {
   RunTasks();
   AnimateSprites();
   
   HiColor_CB2();
   
   BuildOamBuffer();
   UpdatePaletteFade();
}
static void VBlankCB(void) {
   HiColor_VBlank();
   LoadOam();
   ProcessSpriteCopyRequests();
   TransferPlttBuffer();
}
static void VBlankCB_BGPalOnly(void) {
   HiColor_VBlank();
   LoadOam();
   ProcessSpriteCopyRequests();
   DmaCopy16(3, gPlttBufferFaded, (void*)BG_PLTT, BG_PLTT_SIZE);
}

// ---------------------------------------------------------

static void TestSpecificFrameHandler(void) {
}
static void TestShutdownHandler(void) {
   SetHBlankCallback(NULL);
   HiColor_Reset();
}

#define SOLID_BARS_2_COLORS(a, b) \
   { \
      RGB(31, 0,31), \
      (a), (a), (a), (a), (a), (a), (a), \
      (((a) + (b)) / 2), \
      (b), (b), (b), (b), (b), (b), (b) \
   }
#define SOLID_BARS_3_COLORS(a, b, c) \
   { \
      RGB(31, 0,31), \
      (a), (a), (a), (a), (a), \
      (b), (b), (b), (b), (b), \
      (c), (c), (c), (c), (c), \
   }
#define SOLID_BARS_4_COLORS(a, b, c, d) \
   { \
      RGB(31, 0,31), \
      (a), (a), (a), (a), \
      (b), (b), (b), (b), \
      (c), (c), (c), (c), \
      (d), (d), (d), \
   }
#define SOLID_BARS_5_COLORS(a, b, c, d, e) \
   { \
      RGB(31, 0,31), \
      (a), (a), (a), \
      (b), (b), (b), \
      (c), (c), (c), \
      (d), (d), (d), \
      (e), (e), (e), \
   }

ALIGNED(4) typedef u16 PaletteAligned32 [16];

static const PaletteAligned32 sSpritePalettes[32] = {
   { // 00: red and blue (each darkening toward center)
      RGB(31, 0,31), // 0 (magenta transparent)
      RGB(31, 0, 0),
      RGB(27, 0, 0),
      RGB(24, 0, 0),
      RGB(22, 0, 0),
      RGB(19, 0, 0),
      RGB(17, 0, 0),
      RGB(15, 0, 0),
      RGB(15, 0,15), // 8
      RGB( 0, 0,15),
      RGB( 0, 0,17),
      RGB( 0, 0,19),
      RGB( 0, 0,22),
      RGB( 0, 0,24),
      RGB( 0, 0,27),
      RGB( 0, 0,31)  // 15
   },
   { // 01: pink and green (each darkening toward center)
      RGB(31, 0,31), // 0 (magenta transparent)
      RGB(31,23,31),
      RGB(29,21,29),
      RGB(24,20,24),
      RGB(22,18,22),
      RGB(19,15,19),
      RGB(17,13,17),
      RGB(15,11,15),
      RGB(15,15,15), // 8
      RGB( 0,15, 0),
      RGB( 0,17, 0),
      RGB( 0,19, 0),
      RGB( 0,22, 0),
      RGB( 0,24, 0),
      RGB( 0,27, 0),
      RGB( 0,31, 0)  // 15
   },
   { // 02: brown and teal (each darkening toward center)
      RGB(31, 0,31), // 0 (magenta transparent)
      RGB(20,13, 0),
      RGB(19,12, 0),
      RGB(18,11, 0),
      RGB(17,10, 4),
      RGB(17,10, 0),
      RGB(16, 9, 0),
      RGB(15, 8, 0),
      RGB(15,15,15), // 8
      RGB( 0,15,15),
      RGB( 0,16,16),
      RGB( 4,17,17),
      RGB( 0,18,18),
      RGB( 4,18,18),
      RGB( 0,19,19),
      RGB( 0,20,20)  // 15
   },
   SOLID_BARS_3_COLORS( // 03: cyan, yellow, magenta
      RGB( 0,31,31),
      RGB(31,31, 0),
      RGB(31, 0,31)
   ),
   SOLID_BARS_3_COLORS( // 04: orchid, gold, lime
      RGB(10, 0,31),
      RGB(31,27,15),
      RGB( 0,31, 0)
   ),
   SOLID_BARS_3_COLORS( // 05: light grey, mid grey, dark grey
      RGB(29,29,29),
      RGB(15,15,15),
      RGB( 7, 7, 7)
   ),
   SOLID_BARS_3_COLORS( // 06: pink, light grey, sky blue
      RGB(31,23,31),
      RGB(28,28,28),
      RGB(17,26,31)
   ),
   SOLID_BARS_3_COLORS( // 07: dark grey, light grey, purple
      RGB(14,14,14),
      RGB(22,22,22),
      RGB(15, 0,15)
   ),
   SOLID_BARS_3_COLORS( // 08: red-purple, purple, blue
      RGB(26, 0,14),
      RGB(14, 9,18),
      RGB( 0, 7,20)
   ),
   SOLID_BARS_3_COLORS( // 09: peach, white, dark pink
      RGB(31,18,10),
      RGB(30,30,30),
      RGB(25,12,20)
   ),
   SOLID_BARS_3_COLORS( // 10: yellow, grey, purple
      RGB(31,31, 4),
      RGB(31,31,28),
      RGB(19,11,25)
   ),
   SOLID_BARS_3_COLORS( // 11: hot pink, gold, turquoise
      RGB(31, 4,17),
      RGB(31,26, 0),
      RGB( 4,22,31)
   ),
   SOLID_BARS_3_COLORS( // 12: green, white, dark grey
      RGB( 7,20, 8),
      RGB(29,29,29),
      RGB(10,10,10)
   ),
   SOLID_BARS_3_COLORS( // 13: purple, white, green
      RGB(22,15,27),
      RGB(29,29,29),
      RGB( 9,15, 4)
   ),
   SOLID_BARS_4_COLORS( // 14: cyan, magenta, yellow, black
      RGB( 4,22,31),
      RGB(31, 4,17),
      RGB(31,31, 0),
      RGB( 3, 3, 3)
   ),
   SOLID_BARS_4_COLORS( // 15: red, red-grey, blue, blue-grey
      RGB(27, 4, 4),
      RGB(27,18,18),
      RGB( 4, 4,27),
      RGB(18,18,27)
   ),
   SOLID_BARS_4_COLORS( // 16: red, orange, yellow, olive
      RGB(31, 0, 0),
      RGB(31,15, 0),
      RGB(31,31, 0),
      RGB(20,26, 9)
   ),
   SOLID_BARS_4_COLORS( // 17: rust, underwater, sage, steel
      RGB(22, 8, 6),
      RGB( 6,13,22),
      RGB( 6,15, 4),
      RGB(20,22,26)
   ),
   SOLID_BARS_4_COLORS( // 18: mint, mustard, puke, mold
      RGB(18,25,23),
      RGB(25,23,18),
      RGB(15,14,11),
      RGB(11,14,15)
   ),
   SOLID_BARS_2_COLORS( // 19: dark pink, salmon
      RGB(12,11,15),
      RGB(26,16,19)
   ),
   SOLID_BARS_2_COLORS( // 20: brown, sky blue
      RGB(14, 9, 5),
      RGB(10,21,27)
   ),
   SOLID_BARS_2_COLORS( // 21: teal, gold
      RGB( 7,18,21),
      RGB(25,22, 8)
   ),
   SOLID_BARS_2_COLORS( // 22: pink cloud, red-purple
      RGB(25,22,28),
      RGB(25, 2,28)
   ),
   SOLID_BARS_2_COLORS( // 23: navy, silver
      RGB( 2, 9,28),
      RGB(20,22,28)
   ),
   SOLID_BARS_2_COLORS( // 24: sick green, flesh
      RGB(20,22, 2),
      RGB(28,22,21)
   ),
   SOLID_BARS_2_COLORS( // 25: dark teal, deep blue
      RGB( 2, 9,13),
      RGB( 2, 9,27)
   ),
   SOLID_BARS_2_COLORS( // 26: cream, coffee
      RGB(27,26,25),
      RGB(20,17,12)
   ),
   SOLID_BARS_5_COLORS( // 27: sky blue, pink, light grey, pink, sky blue
      RGB(17,26,31),
      RGB(31,23,31),
      RGB(28,28,28),
      RGB(31,23,31),
      RGB(17,26,31)
   ),
   SOLID_BARS_5_COLORS( // 28: red, yellow, green, blue, purple
      RGB(31, 0, 0),
      RGB(31,27, 0),
      RGB( 0,31, 7),
      RGB( 0,15,31),
      RGB(26, 0,31)
   ),
   SOLID_BARS_5_COLORS( // 29: purple to white
      RGB(26, 0,31),
      RGB(26, 7,31),
      RGB(26,14,31),
      RGB(26,21,31),
      RGB(29,29,29)
   ),
   SOLID_BARS_5_COLORS( // 30: red to yellow
      RGB(31, 0, 0),
      RGB(31, 7, 0),
      RGB(31,15, 0),
      RGB(31,22, 0),
      RGB(31,31, 0)
   ),
   SOLID_BARS_5_COLORS( // 31: yellow to lime
      RGB(31,31, 0),
      RGB(22,31, 0),
      RGB(15,31, 0),
      RGB( 7,31, 0),
      RGB( 0,31, 0)
   ),
};