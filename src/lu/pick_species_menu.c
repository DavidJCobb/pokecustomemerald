#include "global.h"
#include "lu/pick_species_menu.h"

#include "bg.h"
#include "decompress.h" // LoadCompressedSpriteSheet
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h" // AllocZeroed, Free
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h" // PlaySE
#include "sprite.h"
#include "strings.h" // literally just for the "Cancel" option lol
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "gba/m4a_internal.h"
#include "constants/rgb.h"
#include "constants/songs.h" // SE_SELECT and other sound effect constants

#include "constants/characters.h"
#include "lu/widgets/enum_picker.h"
#include "lu/widgets/keybind_strip.h"
#include "lu/widgets/scrollbar_v.h"
#include "lu/string_wrap.h"
#include "lu/strings.h"
#include "lu/ui_helpers.h"
#include "lu/vram_layout_helpers.h"

enum {
   LISTING_POS_FIRST_LETTER,
   LISTING_POS_TYPE_1,
   LISTING_POS_TYPE_2,
   LISTING_POS_GENERATION,
   LISTING_POS_SWITCH_FOCUS_TO_RESULTS,
};

struct MenuState {
   u8  cursor_is_in_results;
   u16 cursor_pos;
   u8  sprite_id_value_arrow_l;
   u8  sprite_id_value_arrow_r;
   struct {
      struct LuEnumPicker   enum_picker;   // for filtering options
      struct LuKeybindStrip keybind_strip;
      struct LuScrollbar    scrollbar;     // for results listing
   } widgets;
};
static EWRAM_DATA struct MenuState* sMenuState = NULL;

static const u8 sMenuCursorBGTiles[] = INCBIN_U8("graphics/lu/cgo_menu/bg-tiles-cursor.4bpp");
static const u8 sBlankBGTile[] = INCBIN_U8("graphics/lu/cgo_menu/bg-tile-blank.4bpp"); // color 1

static const u16 sOptionsListingPalette[] = INCBIN_U16("graphics/lu/cgo_menu/option_listing.gbapal");
//
// Background, foreground, shadow:
//
static const u8 sTextColor_OptionNames[] = {1, 2, 3};
static const u8 sTextColor_OptionValues[] = {1, 4, 5};

static void CreateInterfaceSprites(void);
static void PositionValueArrowsAtRow(u8 screen_row);

static void Task_MenuFadeIn(u8 taskId);
static void Task_MenuProcessInput(u8 taskId);
static void Task_MenuFadeOut(u8 taskId);

enum {
   WIN_HEADER,
   WIN_FORM,
   WIN_LISTING,
   
   WIN_COUNT
};

#define SCROLLBAR_PALETTE_INDEX_BLANK 1
#define SCROLLBAR_PALETTE_INDEX_TRACK 14
#define SCROLLBAR_PALETTE_INDEX_THUMB 15

#define BACKGROUND_LAYER_NORMAL 0

#define BACKGROUND_PALETTE_ID_MENU 0
#define BACKGROUND_PALETTE_ID_TEXT 1
#define BACKGROUND_PALETTE_ID_CONTROLS 2
#define BACKGROUND_PALETTE_BOX_FRAME   7

#define TEXT_ROW_HEIGHT_IN_TILES   2

#define WIN_HEADER_TILE_WIDTH    DISPLAY_TILE_WIDTH
#define WIN_HEADER_TILE_HEIGHT   TEXT_ROW_HEIGHT_IN_TILES

#define WIN_FORM_TILE_X      2
#define WIN_FORM_TILE_Y      (WIN_HEADER_TILE_HEIGHT + 1)
#define WIN_FORM_TILE_WIDTH  (DISPLAY_TILE_WIDTH / 2)
#define WIN_FORM_TILE_HEIGHT (DISPLAY_TILE_HEIGHT - WIN_FORM_TILE_Y - KEYBIND_STRIP_TILE_HEIGHT)

#define WIN_LISTING_TILE_X      (WIN_FORM_TILE_X + WIN_FORM_TILE_WIDTH)
#define WIN_LISTING_TILE_Y      WIN_FORM_TILE_Y
#define WIN_LISTING_TILE_WIDTH  (DISPLAY_TILE_WIDTH - WIN_LISTING_TILE_X - 1)
#define WIN_LISTING_TILE_HEIGHT WIN_FORM_TILE_HEIGHT

// Never instantiated. Just a marginally less hideous way to manage all this 
// compared to preprocessor macros. Unit of measurement is 4bpp tile IDs.
typedef struct {
   VRAMTile transparent_tile;
   VRAMTile blank_tile;
   VRAMTile selection_cursor_tiles[4];
   VRAMTile user_window_frame[9];
   VRAMTile scrollbar_tiles[SCROLLBAR_TILE_COUNT];
   
   VRAMTilemap tilemaps[4];
   
   VRAMTile win_tiles_for_header[WIN_HEADER_TILE_WIDTH * WIN_HEADER_TILE_HEIGHT];
   VRAMTile win_tiles_for_keybinds[KEYBIND_STRIP_TILE_COUNT];
   VRAMTile win_tiles_for_form[WIN_FORM_TILE_WIDTH * WIN_FORM_TILE_HEIGHT];
   VRAMTile win_tiles_for_listing[WIN_LISTING_TILE_WIDTH * WIN_LISTING_TILE_HEIGHT];
} VRAMTileLayout;
STATIC_ASSERT(sizeof(VRAMTileLayout) <= BG_VRAM_SIZE, sStaticAssertion01_VramUsage);

static const struct LuKeybindStripEntry sKeybindStripEntries[] = {
   {
      .buttons = (1 << CHAR_DPAD_UPDOWN),
      .text    = gText_lu_UI_KeybindPick
   },
   {
      .buttons = (1 << CHAR_DPAD_LEFTRIGHT),
      .text    = gText_lu_UI_KeybindChange
   },
   {
      .buttons = (1 << CHAR_A_BUTTON),
      .text    = gText_lu_UI_KeybindViewSearchResults
   },
   {
      .buttons = (1 << CHAR_A_BUTTON),
      .text    = gText_lu_UI_KeybindChooseSpecies
   },
   {
      .buttons = (1 << CHAR_B_BUTTON),
      .text    = gText_lu_UI_KeybindBack
   },
};
static const struct LuKeybindStripInitParams sKeybindStripInit = {
   .bg_layer      = BACKGROUND_LAYER_NORMAL,
   .first_tile_id = VRAM_BG_TileID(VRAMTileLayout, win_tiles_for_keybinds),
   .palette_id    = BACKGROUND_PALETTE_ID_CONTROLS
};

static const struct LuScrollbarGraphicsParams sScrollbarInit = {
   .bg_layer      = BACKGROUND_LAYER_NORMAL,
   .palette_id    = BACKGROUND_PALETTE_ID_TEXT,
   .color_track   = SCROLLBAR_PALETTE_INDEX_TRACK,
   .color_thumb   = SCROLLBAR_PALETTE_INDEX_THUMB,
   .color_blank   = SCROLLBAR_PALETTE_INDEX_BLANK,
   .pos_x         = WIN_LISTING_TILE_X + WIN_LISTING_TILE_WIDTH,
   .pos_y         = WIN_LISTING_TILE_Y,
   .length        = WIN_LISTING_TILE_HEIGHT,
   .first_tile_id = VRAM_BG_TileID(VRAMTileLayout, scrollbar_tiles)
};

static const struct BgTemplate sOptionMenuBgTemplates[] = {
   {
      .bg = BACKGROUND_LAYER_NORMAL,
      //
      .charBaseIndex = 0,
      .mapBaseIndex  = VRAM_BG_MapBaseIndex(VRAMTileLayout, tilemaps[BACKGROUND_LAYER_NORMAL]),
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 2,
      .baseTile      = 0
   },
};

static const struct WindowTemplate sOptionMenuWinTemplates[] = {
    [WIN_HEADER] = {
        .bg          = BACKGROUND_LAYER_NORMAL,
        .tilemapLeft = 0,
        .tilemapTop  = 0,
        .width       = WIN_HEADER_TILE_WIDTH,
        .height      = WIN_HEADER_TILE_HEIGHT,
        .paletteNum  = BACKGROUND_PALETTE_ID_CONTROLS,
        .baseBlock   = VRAM_BG_TileID(VRAMTileLayout, win_tiles_for_header)
    },
    [WIN_FORM] = {
        .bg          = BACKGROUND_LAYER_NORMAL,
        .tilemapLeft = WIN_FORM_TILE_X,
        .tilemapTop  = WIN_FORM_TILE_Y,
        .width       = WIN_FORM_TILE_WIDTH,
        .height      = WIN_FORM_TILE_HEIGHT,
        .paletteNum  = BACKGROUND_PALETTE_ID_TEXT,
        .baseBlock   = VRAM_BG_TileID(VRAMTileLayout, win_tiles_for_form)
    },
    [WIN_LISTING] = {
        .bg          = BACKGROUND_LAYER_NORMAL,
        .tilemapLeft = WIN_LISTING_TILE_X,
        .tilemapTop  = WIN_LISTING_TILE_Y,
        .width       = WIN_LISTING_TILE_WIDTH,
        .height      = WIN_LISTING_TILE_HEIGHT,
        .paletteNum  = BACKGROUND_PALETTE_ID_TEXT,
        .baseBlock   = VRAM_BG_TileID(VRAMTileLayout, win_tiles_for_listing)
    },
    //
    [WIN_COUNT] = DUMMY_WIN_TEMPLATE
};

static const u16 sOptionMenuBg_Pal[] = {RGB(17, 18, 31)};

//
//
//

static void MainCB2(void) {
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

