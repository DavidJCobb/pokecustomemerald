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

struct MenuState {
   void(*callback)(void);
   u8    task_id;
   bool8 initialized;
   u8    timer;
   u8    counter;
   
   u8 tile_src_buffer[TILE_SIZE_4BPP * (128 / TILE_WIDTH) * (32 / TILE_HEIGHT)];
};
static EWRAM_DATA struct MenuState* sMenuState = NULL;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Task_FrameHandler(u8);
static void CB2_Init(void);
static void CB2_Idle(void);
static void VBlankCB(void);
static void TestSpecificFrameHandler(void);

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
         gMain.state++;
         break;
         
      //
      // Code here is specific to each test, and should set up the appropriate 
      // graphics resources.
      //
      case 1:
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
         V_LOAD_TILES(0, dpad_tiles, sKeypadIconTiles);
         gMain.state++;
         break;
         
      default:
         SetVBlankCallback(VBlankCB);
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
   BuildOamBuffer();
   UpdatePaletteFade();
}
static void VBlankCB(void) {
   LoadOam();
   ProcessSpriteCopyRequests();
   TransferPlttBuffer();
}

// ---------------------------------------------------------

static void TestSpecificFrameHandler(void) {
   if (!(sMenuState->timer % 60)) {
      sMenuState->timer = 0;
      sMenuState->counter = (sMenuState->counter + 1) % 16;
      
      //
      // BG tiles are "compiled" in row-major order.
      //
      
      struct Bitmap src_bitmap = {
         .pixels = sKeypadIconTiles,
         .width  = 128,
         .height =  32,
      };
      struct Bitmap dst_bitmap = {
         .pixels = sMenuState->tile_src_buffer,
         .width  = 128,
         .height =  32,
      };
      
      u8 remapping[16] = {};
      for(u8 i = 1; i < 16; ++i) {
         remapping[i] = (sMenuState->counter + i) % 16;
      }
      
      memcpy(sMenuState->tile_src_buffer, sKeypadIconTiles, sizeof(sKeypadIconTiles));
      
      const u8* src = sKeypadIconTiles;
      u8*       dst = sMenuState->tile_src_buffer;
      {  // B-button.
         u8 remapping[16] = {};
         for(u8 i = 1; i < 16; ++i) {
            remapping[i] = (sMenuState->counter + i + 1) % 16;
         }
         
         const u8 tile_ids[] = { 1, 17 };
         for(u8 i = 0; i < ARRAY_COUNT(tile_ids); ++i)
            BlitTile4BitRemapped(&src[tile_ids[i] * TILE_SIZE_4BPP], &dst[tile_ids[i] * TILE_SIZE_4BPP], remapping);
      }
      
      // Recolor "L" button
      BlitBitmapRect4BitRemapped(
         &src_bitmap,
         &dst_bitmap,
         17, 0,
         17, 0,
         15, 16,
         remapping
      );
      
      // Recolor middle of "R" button
      BlitBitmapRect4BitRemapped(
         &src_bitmap,
         &dst_bitmap,
         35, 0,
         35, 0,
         10, 16,
         remapping
      );
      
      // Recolor D-Pad
      BlitBitmapRect4BitRemapped(
         &src_bitmap,
         &dst_bitmap,
         96, 0,
         96, 0,
         32, 16,
         remapping
      );
      BlitBitmapRect4BitRemapped(
         &src_bitmap,
         &dst_bitmap,
         0, 16,
         0, 16,
         24, 16,
         remapping
      );
      
      V_LOAD_TILES(0, recolored_tiles, sMenuState->tile_src_buffer);
      
      // Draw original tiles.
      {
         u16 base_id = V_TILE_ID(dpad_tiles);
         u8  w = 128 / TILE_WIDTH;
         u8  h =  32 / TILE_HEIGHT;
         for(u8 y = 0; y < h; ++y) {
            for(u8 x = 0; x < w; ++x) {
               u8 id = (y * w) + x;
               FillBgTilemapBufferRect(0, base_id + id, x, y + h, 1, 1, 0);
            }
         }
      }
      
      u16 base_id = V_TILE_ID(recolored_tiles);
      u8  w = 128 / TILE_WIDTH;
      u8  h =  32 / TILE_HEIGHT;
      for(u8 y = 0; y < h; ++y) {
         for(u8 x = 0; x < w; ++x) {
            u8 id = (y * w) + x;
            FillBgTilemapBufferRect(0, base_id + id, x, y, 1, 1, 0);
         }
      }
      CopyBgTilemapBufferToVram(0);
   }
}