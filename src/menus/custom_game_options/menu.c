#include "global.h"
#include "menus/custom_game_options/menu.h"
#include "custom_game_options/options.h"

#include "bg.h"
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

#include "graphics.h" // interface sprites

#include "constants/characters.h"
#include "string_util.h"
#include "lu/widgets/enum_picker.h"
#include "lu/widgets/keybind_strip.h"
#include "lu/widgets/num_edit_modal.h"
#include "lu/widgets/scrollbar_v.h"
#include "lu/string_wrap.h"
#include "lu/strings.h"
#include "lu/ui_helpers.h"
#include "lu/vram_layout_helpers.h"

#include "lu/pick_species_menu.h"

/*//

   The current menu design is as follows:
   
    - Most of the GUI is on a shared background layer.
    
    - The help text is on its own background layer as well, set to display overtop 
      all others. The help text window is sized so that it doesn't cover the menu 
      header or keybind strip, so those always show through.
   
   
   
   Potential design improvements:
   
    - It'd be nice if pressing A on an integral option would open a small editor 
      allowing you to type in a new numeric value. We could display the editor 
      directly overtop the option value, with a 3x4 grid of digits to enter (0-9) 
      and OK/Cancel buttons you can navigate to. We'd have to implement a usable 
      cursor, among other things.
      
      I feel like this might be a useful UI to implement as a general-purpose 
      thing: the ability to call a function and say, "Hey, buddy, spawn this 
      widget at W screen offset on X background layer, and use Y task ID until 
      you're done; then run Z callback." We could put this widget on the Help 
      layer.
      
       - Being able to press A on enum options to get a drop-down would be nice, 
         too.
   
    - Submenus should have some kind of icon like ">>" shown where a value would 
      be.
      
    - We need theming in general but especially for the Help page.
      
    - It'd be nice if the Help text were scrollable, so we wouldn't have to 
      worry about it being too long to display. Maybe we could enable Window 1 
      and use it for the help screen: set the help background layer to a size of 
      256x512px, and only enable it within LCD Window 1, not LCD Window 0 or the 
      Not-in-Window region; then, we can modify the layer's position to scroll it, 
      with LCD Window 1 cropping it.
   
    - An enum option can be defined to have help text for the option as a whole, 
      and for each individual value. However, we only display one. It'd be nice 
      if we could draw both, though maybe that should require a MENUITEM_FLAG_.
      
       - Let's hold off on this until we actually have enough enum options to 
         see if it'd be terribly helpful.
   
    - We need a separate UI for rendering Pokemon species selection, with the 
      ability to search by type and by name substring. This is required in order 
      to implement options that let you pick a species, like overriding the game's 
      starter Pokemon, or forcing single-species runs.
      
       - The Pokedex has filter-and-sort functionality, though the actual sort 
         and filter criteria available are very limited. It'd be useful to see 
         just how they implemented generating the list of species to display. 
         Their actual search form, however, isn't worth copying; we should design 
         our own.
         
          - Since they use a heap-allocated struct for the Pokedex UI state, they 
            literally just have an array of dex list items (dex number, seen flag, 
            and owned flag) as a member, with extent `NATIONAL_DEX_COUNT + 1`. 
            Guess we'll do the same, then.

//*/

enum {
   WIN_HEADER,
   WIN_OPTIONS,
   WIN_HELP,
   //
   WIN_COUNT
};

enum {
   MENUVIEW_OPENING,
   MENUVIEW_OPEN,
   MENUVIEW_IN_HELP,
   MENUVIEW_IN_SUBSCREEN,
   MENUVIEW_RETURNING_FROM_SUBSCREEN,
};

#define SOUND_EFFECT_MENU_SCROLL   SE_SELECT
#define SOUND_EFFECT_SUBMENU_ENTER SE_SELECT
#define SOUND_EFFECT_SUBMENU_EXIT  SE_SELECT
#define SOUND_EFFECT_HELP_EXIT     SE_SELECT

EWRAM_DATA static struct CustomGameOptionsSet sTempOptions;

#include "menus/custom_game_options/menu_item.h"
#include "menus/custom_game_options/menu_hierarchy_definition.h" // sTopLevelMenu is the top-level menu

//
// Menu state and associated funcs:
//

#define MAX_MENU_TRAVERSAL_DEPTH 8

struct MenuStackFrame {
   const u8* name;
   const struct CGOptionMenuItem* menu_items;
};

struct MenuState {
   struct MenuStackFrame breadcrumbs[MAX_MENU_TRAVERSAL_DEPTH];
   u8 saved_cursor_positions[MAX_MENU_TRAVERSAL_DEPTH];
   
   u8 task_id;
   u8 cursor_pos;
   u8 menu_view;
   u8 help_window_id;
   struct {
      struct LuEnumPicker   enum_picker;
      struct LuKeybindStrip keybind_strip;
      struct LuNumEditModal num_edit_modal;
      struct LuScrollbar    scrollbar;
   } widgets;
};

static EWRAM_DATA struct MenuState* sMenuState = NULL;

static void ResetMenuState(void);

static void UpdateDisplayedMenuName(void);
static void UpdateDisplayedMenuItems(void);
static u8 GetScreenRowForCursorPos(void);

static const struct CGOptionMenuItem* GetCurrentMenuItemList(void);

static void EnterSubmenu(const u8* submenu_name, const struct CGOptionMenuItem* submenu_items);
static bool8 TryExitSubmenu();

static void TryMoveMenuCursor(s8 by);

//
// Sprites:
//

#define SPRITE_TAG_ENUM_PICKER 4096

//
// Task and drawing stuff:
//

static void TearDownVisuals(void);

static void Task_CGOptionMenuFadeIn(u8 taskId);
static void Task_CGOptionMenuProcessInput(u8 taskId);
static void Task_CGOptionMenuWaitForSubscreen(u8 taskId);
static void Task_CGOptionMenuSave(u8 taskId);
static void Task_CGOptionMenuFadeOut(u8 taskId);
static void HighlightCGOptionMenuItem();

static void OpenSubscreen_PickSpeciesMenu(void);
static void OnSubscreenClosed_PickSpeciesMenu(PokemonSpeciesID);

static void OpenNumEditModal(LuNumEditModalValue min, LuNumEditModalValue max, LuNumEditModalValue current);
static void Task_CGOptionMenuWaitForNumEditModal(u8 taskId);
static void OpenNumEditModal_Callback(bool8, LuNumEditModalValue);

static void TryDisplayHelp(const struct CGOptionMenuItem* item);
static void TryCloseHelp(void);

static void UpdateDisplayedMenuName(void);
static void DrawMenuItem(const struct CGOptionMenuItem* item, u8 row, bool8 is_single_update);
static void UpdateDisplayedMenuItems(void);
static void RepaintScrollbar(void);
static void UpdateDisplayedControls(void);

//

static const u8 sMenuCursorBGTiles[] = INCBIN_U8("graphics/lu/cgo_menu/bg-tiles-cursor.4bpp");
static const u8 sBlankBGTile[] = INCBIN_U8("graphics/lu/cgo_menu/bg-tile-blank.4bpp"); // color 1

static const u16 sOptionsListingPalette[] = INCBIN_U16("graphics/lu/cgo_menu/option_listing.gbapal");
//
// Background, foreground, shadow:
//
static const u8 sTextColor_OptionNames[] = {1, 2, 3};
static const u8 sTextColor_OptionValues[] = {1, 4, 5};
static const u8 sTextColor_HelpBodyText[] = {1, 2, 3};

// BG palette for options window area:
//  0 transparent
//  1 background color (white)
//  2 option name text
//  3 option name shadow
//  4 option value text
//  5 option value shadow
//  6 ?
//  7 ?
//  8 ?
//  9 ?
// 14 scrollbar track
// 15 scrollbar thumb
#define SCROLLBAR_PALETTE_INDEX_BLANK 1
#define SCROLLBAR_PALETTE_INDEX_TRACK 14
#define SCROLLBAR_PALETTE_INDEX_THUMB 15

#define BACKGROUND_LAYER_NORMAL  0
#define BACKGROUND_LAYER_OPTIONS 1
#define BACKGROUND_LAYER_OVERLAY 2 // help, modals

#define BACKGROUND_PALETTE_ID_MENU     0
#define BACKGROUND_PALETTE_ID_TEXT     1
#define BACKGROUND_PALETTE_ID_CONTROLS 2
#define BACKGROUND_PALETTE_BOX_FRAME   7

#define TEXT_ROW_HEIGHT_IN_TILES   2

#define WIN_HEADER_TILE_WIDTH    DISPLAY_TILE_WIDTH
#define WIN_HEADER_TILE_HEIGHT   TEXT_ROW_HEIGHT_IN_TILES
//
#define WIN_OPTIONS_X_TILES             2
#define WIN_OPTIONS_Y_TILES             (WIN_HEADER_TILE_HEIGHT + 1)
#define OPTIONS_INSET_RIGHT_TILES       1
#define OPTIONS_LIST_INSET_RIGHT        (OPTIONS_INSET_RIGHT_TILES * TILE_WIDTH)
#define OPTIONS_LIST_ROW_HEIGHT         7
//
#define WIN_OPTIONS_TILE_WIDTH    (DISPLAY_TILE_WIDTH - WIN_OPTIONS_X_TILES - 1)
#define WIN_OPTIONS_TILE_HEIGHT   (OPTIONS_LIST_ROW_HEIGHT * TEXT_ROW_HEIGHT_IN_TILES)
//
#define WIN_HELP_TILE_WIDTH    DISPLAY_TILE_WIDTH
#define WIN_HELP_TILE_HEIGHT   (DISPLAY_TILE_HEIGHT - WIN_HEADER_TILE_HEIGHT - KEYBIND_STRIP_TILE_HEIGHT)
//
#define MAX_MENU_ITEMS_VISIBLE_AT_ONCE   OPTIONS_LIST_ROW_HEIGHT
#define MENU_ITEM_HALFWAY_ROW            (MAX_MENU_ITEMS_VISIBLE_AT_ONCE / 2)

#define OPTION_VALUE_COLUMN_WIDTH  72
#define OPTION_VALUE_COLUMN_X      ((WIN_OPTIONS_TILE_WIDTH * TILE_WIDTH) - OPTIONS_LIST_INSET_RIGHT - OPTION_VALUE_COLUMN_WIDTH)


#define BG_LAYER_HELP_TILESET_INDEX 2
#define BG_LAYER_WIDGETS_TILESET_INDEX 3

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
   VRAMTile win_tiles_for_options[WIN_OPTIONS_TILE_WIDTH * WIN_OPTIONS_TILE_HEIGHT];
   
   // We have the help screen set to use Tileset 2, so it can address tiles in the range [1536, 2047].
   // We want to skip a tile so that we have a blank/transparent tile we can display.
   VRAMTile VRAM_BG_AT_CHAR_BASE_INDEX(BG_LAYER_HELP_TILESET_INDEX) blank_tile_for_help;
   union {
      VRAMTile win_tiles_for_help[WIN_HELP_TILE_WIDTH * WIN_HELP_TILE_HEIGHT];
      struct {
         VRAMTile num_edit_modal_border[9];
         VRAMTile num_edit_modal_body[NUM_EDIT_MODAL_INNER_TILE_WIDTH_MAX * NUM_EDIT_MODAL_INNER_TILE_HEIGHT];
      }; // LuNumEditModal
   };
} VRAMTileLayout;
//
// ensure we fit within 64KB VRAM limit.
STATIC_ASSERT(sizeof(VRAMTileLayout) <= BG_VRAM_SIZE, sStaticAssertion01_VramUsage);

enum {
   KEYBIND_INDEX_PICK,
   KEYBIND_INDEX_CHANGE,
   KEYBIND_INDEX_TYPE_IN,
   KEYBIND_INDEX_ENTER_SUBMENU,
   KEYBIND_INDEX_RETURN_TO_MENU,
   KEYBIND_INDEX_BACK,
   KEYBIND_INDEX_HELP,
};

static const struct LuEnumPickerInitParams sEnumPickerInit = {
   .base_pos = {
      .x = (WIN_OPTIONS_X_TILES * TILE_WIDTH) + OPTION_VALUE_COLUMN_X,
      .y = (WIN_OPTIONS_Y_TILES * TILE_HEIGHT),
   },
   .sprite_tags = {
      .tile    = SPRITE_TAG_ENUM_PICKER,
      .palette = SPRITE_TAG_ENUM_PICKER,
   },
   .width = OPTION_VALUE_COLUMN_WIDTH,
};
static const struct LuKeybindStripEntry sKeybindStripEntries[] = {
   [KEYBIND_INDEX_PICK] = {
      .buttons = (1 << CHAR_DPAD_UPDOWN),
      .text    = gText_lu_UI_KeybindPick
   },
   [KEYBIND_INDEX_CHANGE] = {
      .buttons = (1 << CHAR_DPAD_LEFTRIGHT),
      .text    = gText_lu_UI_KeybindChange
   },
   [KEYBIND_INDEX_TYPE_IN] = {
      .buttons = (1 << CHAR_A_BUTTON),
      .text    = gText_lu_UI_KeybindTypeIn
   },
   [KEYBIND_INDEX_ENTER_SUBMENU] = {
      .buttons = (1 << CHAR_A_BUTTON),
      .text    = gText_lu_UI_KeybindEnterSubmenu
   },
   [KEYBIND_INDEX_RETURN_TO_MENU] = {
      .buttons = (1 << CHAR_B_BUTTON),
      .text    = gText_lu_UI_KeybindReturnToMenu
   },
   [KEYBIND_INDEX_BACK] = {
      .buttons = (1 << CHAR_B_BUTTON),
      .text    = gText_lu_UI_KeybindBack
   },
   [KEYBIND_INDEX_HELP] = {
      .buttons = (1 << CHAR_L_BUTTON) | (1 << CHAR_R_BUTTON),
      .text    = gText_lu_UI_KeybindHelp
   },
};
static const struct LuKeybindStripInitParams sKeybindStripInit = {
   .bg_layer      = BACKGROUND_LAYER_NORMAL,
   .first_tile_id = VRAM_BG_TileID(VRAMTileLayout, win_tiles_for_keybinds),
   .palette_id    = BACKGROUND_PALETTE_ID_CONTROLS
};
static const struct LuScrollbarGraphicsParams sScrollbarInit = {
   .bg_layer      = BACKGROUND_LAYER_OPTIONS,
   .palette_id    = BACKGROUND_PALETTE_ID_TEXT,
   .color_track   = SCROLLBAR_PALETTE_INDEX_TRACK,
   .color_thumb   = SCROLLBAR_PALETTE_INDEX_THUMB,
   .color_blank   = SCROLLBAR_PALETTE_INDEX_BLANK,
   .pos_x         = WIN_OPTIONS_X_TILES + WIN_OPTIONS_TILE_WIDTH,
   .pos_y         = WIN_OPTIONS_Y_TILES,
   .length        = WIN_OPTIONS_TILE_HEIGHT,
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
      .priority      = 3,
      .baseTile      = 0
   },
   {
      .bg = BACKGROUND_LAYER_OPTIONS,
      //
      .charBaseIndex = 0,
      .mapBaseIndex  = VRAM_BG_MapBaseIndex(VRAMTileLayout, tilemaps[BACKGROUND_LAYER_OPTIONS]),
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 2,
      .baseTile      = 0
   },
   {
      .bg = BACKGROUND_LAYER_OVERLAY,
      //
      .charBaseIndex = BG_LAYER_HELP_TILESET_INDEX,
      .mapBaseIndex  = VRAM_BG_MapBaseIndex(VRAMTileLayout, tilemaps[BACKGROUND_LAYER_OVERLAY]),
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 1,
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
    [WIN_OPTIONS] = {
        .bg          = BACKGROUND_LAYER_OPTIONS,
        .tilemapLeft = WIN_OPTIONS_X_TILES,
        .tilemapTop  = WIN_OPTIONS_Y_TILES,
        .width       = WIN_OPTIONS_TILE_WIDTH,
        .height      = WIN_OPTIONS_TILE_HEIGHT,
        .paletteNum  = BACKGROUND_PALETTE_ID_TEXT,
        .baseBlock   = VRAM_BG_TileID(VRAMTileLayout, win_tiles_for_options)
    },
    //
    [2] = DUMMY_WIN_TEMPLATE
};
static const struct WindowTemplate sHelpWindowTemplate = {
   .bg          = BACKGROUND_LAYER_OVERLAY,
   .tilemapLeft = 0,
   .tilemapTop  = 2,
   .width       = WIN_HELP_TILE_WIDTH,
   .height      = WIN_HELP_TILE_HEIGHT,
   .paletteNum  = BACKGROUND_PALETTE_ID_TEXT,
   .baseBlock   = VRAM_BG_CharBasedTileID(BG_LAYER_HELP_TILESET_INDEX, VRAMTileLayout, win_tiles_for_help)
};

static const u16 sOptionMenuBg_Pal[] = {RGB(17, 18, 31)};

//
// Menu state and associated funcs:
//

static void ResetMenuState(void) {
   u8 i;
   
   sMenuState->breadcrumbs[0].name       = gText_lu_CGO_menuTitle;
   sMenuState->breadcrumbs[0].menu_items = sTopLevelMenu;
   sMenuState->saved_cursor_positions[0] = 0;
   for(i = 1; i < MAX_MENU_TRAVERSAL_DEPTH; ++i) {
      sMenuState->breadcrumbs[i].name       = NULL;
      sMenuState->breadcrumbs[i].menu_items = NULL;
      sMenuState->saved_cursor_positions[i] = 0;
   }
   sMenuState->cursor_pos = 0;
   sMenuState->menu_view  = MENUVIEW_OPEN;
   sMenuState->help_window_id = WINDOW_NONE;
}

static const struct CGOptionMenuItem* GetCurrentMenuItemList(void) {
   u8 i;
   const struct CGOptionMenuItem* items;
   
   items = sTopLevelMenu;
   for(i = 0; i < MAX_MENU_TRAVERSAL_DEPTH; ++i) {
      if (sMenuState->breadcrumbs[i].menu_items != NULL) {
         items = sMenuState->breadcrumbs[i].menu_items;
      } else {
         break;
      }
   }
   return items;
}

static void EnterSubmenu(const u8* submenu_name, const struct CGOptionMenuItem* submenu_items) {
   u8    i;
   bool8 success;
   
   success = FALSE;
   for(i = 0; i < MAX_MENU_TRAVERSAL_DEPTH; ++i) {
      if (sMenuState->breadcrumbs[i].menu_items == NULL) {
         sMenuState->breadcrumbs[i].name       = submenu_name;
         sMenuState->breadcrumbs[i].menu_items = submenu_items;
         sMenuState->saved_cursor_positions[i] = 0;
         success = TRUE;
         break;
      }
   }
   if (!success) {
      // TODO: DebugPrint
      return;
   }
   
   if (i > 0) {
      sMenuState->saved_cursor_positions[i - 1] = sMenuState->cursor_pos;
   }
   sMenuState->cursor_pos = 0;
   
   PlaySE(SOUND_EFFECT_SUBMENU_ENTER);
   UpdateDisplayedMenuName();
   UpdateDisplayedMenuItems();
}
static bool8 TryExitSubmenu() { // returns FALSE if at top-level menu
   u8    i;
   bool8 success;
   
   success = FALSE;
   for(i = MAX_MENU_TRAVERSAL_DEPTH - 1; i > 0; --i) {
      if (sMenuState->breadcrumbs[i].menu_items != NULL) {
         sMenuState->breadcrumbs[i].name       = NULL;
         sMenuState->breadcrumbs[i].menu_items = NULL;
         sMenuState->saved_cursor_positions[i] = 0;
         success = TRUE;
         break;
      }
   }
   if (!success) {
      return FALSE; // in top-level menu
   }
   
   if (i > 0) {
      sMenuState->cursor_pos = sMenuState->saved_cursor_positions[i - 1];
   } else {
      sMenuState->cursor_pos = 0;
   }
   
   PlaySE(SOUND_EFFECT_SUBMENU_EXIT);
   UpdateDisplayedMenuName();
   UpdateDisplayedMenuItems();
   
   return TRUE;
}

static void TryMoveMenuCursor(s8 by) {
   u8 items_count = GetMenuItemListCount(GetCurrentMenuItemList());
   
   if (by < 0) {
      if (sMenuState->cursor_pos < -by) {
         sMenuState->cursor_pos = items_count - 1;
      } else {
         sMenuState->cursor_pos += by;
      }
   }
   if (by > 0) {
      sMenuState->cursor_pos += by;
      if (sMenuState->cursor_pos >= items_count) {
         sMenuState->cursor_pos = 0;
      }
   }
   PlaySE(SOUND_EFFECT_MENU_SCROLL);
}

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

void CB2_InitCustomGameOptionMenu(void) {
   switch (gMain.state) {
      default:
      case 0:
         SetVBlankCallback(NULL);
         gMain.state++;
         break;
      case 1:
         LuUI_ResetBackgroundsAndVRAM();
         InitBgsFromTemplates(0, sOptionMenuBgTemplates, ARRAY_COUNT(sOptionMenuBgTemplates));
           
         InitWindows(sOptionMenuWinTemplates);
         DeactivateAllTextPrinters();
         
         ShowBg(BACKGROUND_LAYER_NORMAL);
         ShowBg(BACKGROUND_LAYER_OPTIONS);
         ShowBg(BACKGROUND_LAYER_OVERLAY);
         
         SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP); // enable sprite layer
         
         gMain.state++;
         break;
       case 2:
         LuUI_ResetSpritesAndEffects();
         if (sMenuState == NULL) {
            ResetTasks();
         }
         
         VRAM_LoadBGTiles(blank_tile, BACKGROUND_LAYER_OPTIONS, sBlankBGTile);
         VRAM_LoadBGTiles(selection_cursor_tiles, BACKGROUND_LAYER_OPTIONS, sMenuCursorBGTiles);
         
         gMain.state++;
         break;
       case 3:
         LuUI_LoadPlayerWindowFrame(
            BACKGROUND_LAYER_OPTIONS,
            BACKGROUND_PALETTE_BOX_FRAME,
            VRAM_BG_TileID(VRAMTileLayout, user_window_frame)
         );
         gMain.state++;
         break;
       case 4:
         VRAM_LoadBGPaletteData(sOptionMenuBg_Pal, BACKGROUND_PALETTE_ID_MENU);
         gMain.state++;
         break;
       case 5:
         VRAM_LoadBGPaletteData(sOptionsListingPalette, BACKGROUND_PALETTE_ID_TEXT);
         VRAM_LoadBGPalette(GetTextWindowPalette(2), BACKGROUND_PALETTE_ID_CONTROLS);
         gMain.state++;
         break;
       case 6:
         PutWindowTilemap(WIN_HEADER);
         gMain.state++;
         break;
       case 7:
         //PutWindowTilemap(WIN_HELP);
         gMain.state++;
         break;
       case 8:
         PutWindowTilemap(WIN_OPTIONS);
         FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
         CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
         gMain.state++;
         break;
       case 9:
         //
         gMain.state++;
         break;
       case 10:
         {
            if (sMenuState == NULL) {
               u8 taskId = CreateTask(Task_CGOptionMenuFadeIn, 0);
               {
                  sMenuState = AllocZeroed(sizeof(struct MenuState));
                  sMenuState->task_id   = taskId;
                  sMenuState->menu_view = MENUVIEW_OPENING;
                  ResetMenuState();
               }
               sTempOptions = gCustomGameOptions;
            }
            {
               InitEnumPicker(&sMenuState->widgets.enum_picker, &sEnumPickerInit);
            }
            {
               InitKeybindStrip(&sMenuState->widgets.keybind_strip, &sKeybindStripInit);
               sMenuState->widgets.keybind_strip.entries     = sKeybindStripEntries;
               sMenuState->widgets.keybind_strip.entry_count = ARRAY_COUNT(sKeybindStripEntries);
            }
            {
               struct LuScrollbar* scrollbar = &sMenuState->widgets.scrollbar;
               InitScrollbarV(&sMenuState->widgets.scrollbar, &sScrollbarInit);
               scrollbar->max_visible_items = MAX_MENU_ITEMS_VISIBLE_AT_ONCE;
            }
            UpdateDisplayedMenuName();
            UpdateDisplayedMenuItems();
            UpdateDisplayedControls();
            HighlightCGOptionMenuItem();

            CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
            gMain.state++;
         }
         break;
       case 11:
         BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
         SetVBlankCallback(VBlankCB);
         SetMainCallback2(MainCB2);
         if (sMenuState->menu_view == MENUVIEW_IN_SUBSCREEN) {
            sMenuState->menu_view = MENUVIEW_RETURNING_FROM_SUBSCREEN;
         }
         return;
   }
}

static void TearDownVisuals(void) {
   DestroyEnumPicker(&sMenuState->widgets.enum_picker);
   DestroyKeybindStrip(&sMenuState->widgets.keybind_strip);
   DestroyScrollbarV(&sMenuState->widgets.scrollbar);
   FreeAllWindowBuffers();
}

static void Task_CGOptionMenuFadeIn(u8 taskId) {
   if (!gPaletteFade.active)
      gTasks[taskId].func = Task_CGOptionMenuProcessInput;
}

static void Task_CGOptionMenuProcessInput(u8 taskId) {
   if (sMenuState->menu_view == MENUVIEW_IN_HELP) {
      if (JOY_NEW(B_BUTTON)) {
         TryCloseHelp();
         PlaySE(SOUND_EFFECT_HELP_EXIT);
      }
      return;
   }
   
   // NOTE: We have to check these before the A button, because if the user 
   // sets the "L/R Button" option to "L=A", then pressing L counts as a 
   // press of both L and A.
   if (JOY_NEW(L_BUTTON) || JOY_NEW(R_BUTTON)) {
      const struct CGOptionMenuItem* items = GetCurrentMenuItemList();
      TryDisplayHelp(&items[sMenuState->cursor_pos]);
      return;
   }
   
   if (JOY_NEW(A_BUTTON)) {
      const struct CGOptionMenuItem* items;
      const struct CGOptionMenuItem* item;
      
      items = GetCurrentMenuItemList();
      item  = &items[sMenuState->cursor_pos];
      
      if (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) {
         EnterSubmenu(item->name, item->target.submenu);
         UpdateDisplayedControls();
         HighlightCGOptionMenuItem();
         return;
      }
      if (MenuItemTypeIsIntegral(item->value_type)) {
         OpenNumEditModal(
            item->values.integral.min,
            item->values.integral.max,
            GetOptionValue(item)
         );
         return;
      }
      if (item->value_type == VALUE_TYPE_POKEMON_SPECIES) {
         OpenSubscreen_PickSpeciesMenu();
         return;
      }
      
      return;
   }
   if (JOY_NEW(B_BUTTON)) {
      if (TryExitSubmenu()) {
         UpdateDisplayedControls();
         HighlightCGOptionMenuItem();
         return;
      }
      //
      // We're in the top-level menu. Back out.
      //
      gTasks[taskId].func = Task_CGOptionMenuSave;
      return;
   }
   
   // Up/Down: Move cursor, scrolling if necessary
   {
      s8 by = JOY_NEW(DPAD_UP) ? -1 : 0;
      if (!by) {
         by = JOY_NEW(DPAD_DOWN) ? 1 : 0;
      }
      if (by) {
         TryMoveMenuCursor(by);
         UpdateDisplayedMenuItems();
         UpdateDisplayedControls();
         HighlightCGOptionMenuItem();
         return;
      }
   }
   
   // Left/Right: Cycle option value
   {
      s8 by = JOY_REPEAT(DPAD_RIGHT) ? 1 : 0;
      if (!by) {
         by = JOY_REPEAT(DPAD_LEFT) ? -1 : 0;
      }
      
      if (by) {
         u8 row;
         const struct CGOptionMenuItem* items = GetCurrentMenuItemList();
         
         row = GetScreenRowForCursorPos();
         
         CycleOptionSelectedValue(&items[sMenuState->cursor_pos], by);
         DrawMenuItem(&items[sMenuState->cursor_pos], row, TRUE);
         
         if (by < 0) {
            OnEnumPickerDecreased(&sMenuState->widgets.enum_picker);
         } else {
            OnEnumPickerIncreased(&sMenuState->widgets.enum_picker);
         }
         
         return;
      }
   }
}

static void Task_CGOptionMenuWaitForSubscreen(u8 taskId) {
   switch (sMenuState->menu_view) {
      case MENUVIEW_IN_SUBSCREEN:
         break;
      case MENUVIEW_RETURNING_FROM_SUBSCREEN:
         if (!gPaletteFade.active) {
            sMenuState->menu_view = MENUVIEW_OPEN;
            gTasks[taskId].func = Task_CGOptionMenuProcessInput;
         }
         break;
   }
}

static void Task_CGOptionMenuSave(u8 taskId) {
   gCustomGameOptions = sTempOptions;

   BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
   gTasks[taskId].func = Task_CGOptionMenuFadeOut;
}

static void Task_CGOptionMenuFadeOut(u8 taskId) {
   if (!gPaletteFade.active) {
      DestroyTask(taskId);
      TearDownVisuals();
      Free(sMenuState);
      sMenuState = NULL;
      SetMainCallback2(gMain.savedCallback);
   }
}

static void Task_CGOptionMenuFadeOutToSubscreen_PickSpeciesMenu(u8 taskId) {
   if (gPaletteFade.active) {
      return;
   }
   TearDownVisuals();
   
   gTasks[taskId].func = Task_CGOptionMenuWaitForSubscreen;
   
   struct PickSpeciesMenuParams params = {
      .callback    = OnSubscreenClosed_PickSpeciesMenu,
      .header_text = NULL, // TODO: current option name
      .zero_type   = PICKSPECIESMENU_ZEROTYPE_DEFAULT // TODO
   };
   ShowPickSpeciesMenu(&params);
}
static void OpenSubscreen_PickSpeciesMenu(void) {
   sMenuState->menu_view = MENUVIEW_IN_SUBSCREEN;
   BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
   gTasks[sMenuState->task_id].func = Task_CGOptionMenuFadeOutToSubscreen_PickSpeciesMenu;
}
static void OnSubscreenClosed_PickSpeciesMenu(PokemonSpeciesID sel) {
   SetMainCallback2(CB2_InitCustomGameOptionMenu);
   //
   // Save the user's new selection.
   //
   if (sel == PICKSPECIESMENU_RESULT_CANCELED) {
      return;
   }
   const struct CGOptionMenuItem* item;
   const struct CGOptionMenuItem* items = GetCurrentMenuItemList();
   if (items) {
      item = &items[sMenuState->cursor_pos];
      if (item) {
         if (item->value_type != VALUE_TYPE_POKEMON_SPECIES) {
            return;
         }
         if (sel == 0) {
            if ((item->flags & (1 << MENUITEM_FLAG_POKEMON_SPECIES_ALLOW_0)) == 0) {
               return;
            }
         }
         SetOptionValue(item, sel);
      }
   }
}

static void OpenNumEditModal(LuNumEditModalValue min, LuNumEditModalValue max, LuNumEditModalValue current) {
   gTasks[sMenuState->task_id].func = Task_CGOptionMenuWaitForNumEditModal;
   
   SetEnumPickerVisible(&sMenuState->widgets.enum_picker, FALSE);
   
   u8 row = GetScreenRowForCursorPos();
   u8 y   = (row * TEXT_ROW_HEIGHT_IN_TILES) + WIN_OPTIONS_Y_TILES;
   y -= 1;
   
   struct LuNumEditModalInitParams params = {
      .min_value = min,
      .max_value = max,
      .cur_value = current,
      .callback  = OpenNumEditModal_Callback,
      .use_task  = FALSE,
      .window    = {
         .bg_layer      = BACKGROUND_LAYER_OVERLAY,
         .first_tile_id = VRAM_BG_CharBasedTileID(BG_LAYER_HELP_TILESET_INDEX, VRAMTileLayout, num_edit_modal_body),
         .palette_id    = BACKGROUND_PALETTE_ID_TEXT,
         .x             = 21,
         .y             = y,
      },
      .border = {
         .first_tile_id  = VRAM_BG_CharBasedTileID(BG_LAYER_HELP_TILESET_INDEX, VRAMTileLayout, num_edit_modal_border),
         .palette_id     = BACKGROUND_PALETTE_BOX_FRAME,
         .already_loaded = FALSE,
      },
      .text_colors = {
         .back   = 1,
         .text   = 2,
         .shadow = 3,
      },
      .sprite_tags = {
         .cursor = {
            0xFFFF,
            1234
         },
      },
   };
   
   InitNumEditModal(
      &sMenuState->widgets.num_edit_modal,
      &params
   );
   NumEditModalTakeOverKeybindStrip(&sMenuState->widgets.keybind_strip);
}
static void Task_CGOptionMenuWaitForNumEditModal(u8 taskId) {
   HandleNumEditModalInput(&sMenuState->widgets.num_edit_modal);
}
static void OpenNumEditModal_Callback(bool8 accepted, LuNumEditModalValue value) {
   DestroyNumEditModal(&sMenuState->widgets.num_edit_modal);
   sMenuState->widgets.keybind_strip.entries     = sKeybindStripEntries;
   sMenuState->widgets.keybind_strip.entry_count = ARRAY_COUNT(sKeybindStripEntries);
   UpdateDisplayedControls();
   HighlightCGOptionMenuItem(); // update enumpicker arrow visibility
   
   gTasks[sMenuState->task_id].func = Task_CGOptionMenuProcessInput;
   
   if (!accepted)
      return;
   const struct CGOptionMenuItem* item;
   const struct CGOptionMenuItem* items = GetCurrentMenuItemList();
   if (items) {
      item = &items[sMenuState->cursor_pos];
      if (item) {
         switch (item->value_type) {
            case VALUE_TYPE_U8:
            case VALUE_TYPE_U16:
               break;
            default:
               return;
         }
         SetOptionValue(item, value);
         DrawMenuItem(item, GetScreenRowForCursorPos(), TRUE);
      }
   }
}

static void TryDisplayHelp(const struct CGOptionMenuItem* item) {
   const u8* text = NULL;
   
   text = GetRelevantHelpText(item);
   if (text == NULL) {
      //
      // No help text to display.
      //
      return;
   }
   
   sMenuState->menu_view = MENUVIEW_IN_HELP;
   
   u8 window_id = AddWindow(&sHelpWindowTemplate);
   sMenuState->help_window_id = window_id;
   PutWindowTilemap(window_id);
   FillWindowPixelBuffer(window_id, PIXEL_FILL(1));
   
   StringExpandPlaceholders(gStringVar4, text);
   lu_PrepStringWrap(window_id, FONT_NORMAL);
   lu_StringWrap(gStringVar4);
   
   AddTextPrinterParameterized3(window_id, FONT_NORMAL, 2, 1, sTextColor_OptionNames, TEXT_SKIP_DRAW, gStringVar4);
   
   CopyWindowToVram(window_id, COPYWIN_FULL);
   
   SetEnumPickerVisible(&sMenuState->widgets.enum_picker, FALSE);
   UpdateDisplayedControls();
   ShowBg(BACKGROUND_LAYER_OVERLAY);
}
static void TryCloseHelp(void) {
   if (sMenuState->menu_view != MENUVIEW_IN_HELP)
      return;
   
   sMenuState->menu_view = MENUVIEW_OPEN;
   
   ClearWindowTilemap(sMenuState->help_window_id);
   CopyWindowToVram(sMenuState->help_window_id, COPYWIN_MAP);
   RemoveWindow(sMenuState->help_window_id);
   sMenuState->help_window_id = WINDOW_NONE;
   
   HighlightCGOptionMenuItem(); // update enumpicker arrow visibility
   UpdateDisplayedControls();
}

static void HighlightCGOptionMenuItem() {
   const struct CGOptionMenuItem* item;
   const struct CGOptionMenuItem* items = GetCurrentMenuItemList();
   //
   u8 index = GetScreenRowForCursorPos();
   
   // Draw the cursor indicating the currently focused menu item.
   FillBgTilemapBufferRect(
      BACKGROUND_LAYER_NORMAL,
      VRAM_BG_TileID(VRAMTileLayout, blank_tile),
      0,
      WIN_OPTIONS_Y_TILES,
      WIN_OPTIONS_X_TILES, // width to paint over
      WIN_OPTIONS_TILE_HEIGHT,
      BACKGROUND_PALETTE_ID_TEXT
   );
   {
      u8 i;
      u8 x;
      u8 y;
      for(i = 0; i < VRAM_BG_TileCount(VRAMTileLayout, selection_cursor_tiles); ++i) {
         x = i % 2;
         y = i / 2;
         FillBgTilemapBufferRect(
            BACKGROUND_LAYER_NORMAL,
            VRAM_BG_TileID(VRAMTileLayout, selection_cursor_tiles[0]) + i,
            x,
            WIN_OPTIONS_Y_TILES + (index * 2) + y,
            1,
            1,
            BACKGROUND_PALETTE_ID_TEXT
         );
      }
   }
   CopyBgTilemapBufferToVram(BACKGROUND_LAYER_NORMAL);
   
   // For options (i.e. not submenus, etc.), position arrow indicators around the value. 
   // We'll animate them when the player changes the value, for flavor.
   SetEnumPickerRow(&sMenuState->widgets.enum_picker, index);
   
   // Of course, for non-options, we need to *hide* those arrows...
   
   item = NULL;
   if (items) {
      item = &items[sMenuState->cursor_pos];
   }
   
   if (item == NULL) {
      SetEnumPickerVisible(&sMenuState->widgets.enum_picker, FALSE);
      return;
   }
   if (
      (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) == 0 &&
      item->value_type != VALUE_TYPE_POKEMON_SPECIES
   ) {
      SetEnumPickerVisible(&sMenuState->widgets.enum_picker, TRUE);
   } else {
      SetEnumPickerVisible(&sMenuState->widgets.enum_picker, FALSE);
   }
}

static void UpdateDisplayedMenuName(void) {
   u8 i;
   const u8* title;
   
   title = gText_lu_CGO_menuTitle;
   for(i = 0; i < MAX_MENU_TRAVERSAL_DEPTH; ++i) {
      if (sMenuState->breadcrumbs[i].name != NULL) {
         title = sMenuState->breadcrumbs[i].name;
      } else {
         break;
      }
   }
   
   FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
   AddTextPrinterParameterized(WIN_HEADER, FONT_BOLD, title, 8, 1, TEXT_SKIP_DRAW, NULL);
   CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawMenuItem(const struct CGOptionMenuItem* item, u8 row, bool8 is_single_update) {
   u16 value;
   u16 text_width;
   u16 x_offset;
   const u8* value_text;
   
   if (is_single_update) {
      FillWindowPixelRect(
         WIN_OPTIONS,
         PIXEL_FILL(1),
         0,
         (row * 16) + 1,
         
         // Don't paint over the righthand inset, or we'll clobber bits of the scrollbar
         (sOptionMenuWinTemplates[WIN_OPTIONS].width * TILE_WIDTH) - OPTIONS_LIST_INSET_RIGHT,
         
         16
      );
   }
   
   // Name
   AddTextPrinterParameterized3(
      WIN_OPTIONS,
      FONT_NORMAL,
      0, // x
      (row * 16) + 1, // y
      sTextColor_OptionNames,
      TEXT_SKIP_DRAW,
      item->name
   );
   
   // Value
   value      = GetOptionValue(item);
   value_text = GetOptionValueName(item, value);
   if (value_text != NULL) {
      text_width = GetStringWidth(FONT_NORMAL, value_text, 0);
      //
      // let's center the option value within the available space, for now
      // (it'll look neater once we display left/right arrows next to it)
      x_offset = ((s16)OPTION_VALUE_COLUMN_WIDTH - text_width) / 2 + OPTION_VALUE_COLUMN_X;
      //
      AddTextPrinterParameterized3(
         WIN_OPTIONS,
         FONT_NORMAL,
         x_offset,       // x
         (row * 16) + 1, // y
         sTextColor_OptionValues,
         TEXT_SKIP_DRAW,
         value_text
      );
   } else if (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) {
      // TODO: For submenus, draw an icon like ">>" in place of the value
   } else if (item->value_type == VALUE_TYPE_U8 || item->value_type == VALUE_TYPE_U16) {
      
      #define _BUFFER_SIZE 24
      
      u8  text[_BUFFER_SIZE];
      u8  i;
      
      if (value == 0) {
         text[0] = CHAR_0;
         text[1] = EOS;
         if (item->flags & (1 << MENUITEM_FLAG_PERCENTAGE)) {
            text[1] = CHAR_PERCENT;
            text[2] = EOS;
         }
      } else {
         ConvertIntToDecimalStringN(text, value, STR_CONV_MODE_LEFT_ALIGN, 8);
         if (item->flags & (1 << MENUITEM_FLAG_PERCENTAGE)) {
            u16 len = StringLength(text);
            if (len < _BUFFER_SIZE - 2) {
               text[len]     = CHAR_PERCENT;
               text[len + 1] = EOS;
            }
         }
      }
      
      {  // Apply format string.
         const u8* fs = GetOptionValueFormatString(item);
         if (fs) {
            const u8* src = fs;
            u8*       dst = text;
            for(u8 i = 0; i < (_BUFFER_SIZE - 1); ++i, ++dst, ++src) {
               u8 c = *src;
               if (c == CHAR_DYNAMIC) {
                  dst = ConvertIntToDecimalStringN(
                     dst,
                     value,
                     STR_CONV_MODE_LEFT_ALIGN,
                     i + 8 + 1 > _BUFFER_SIZE ? _BUFFER_SIZE - i - 1 : 8
                  );
                  --dst;
                  continue;
               }
               *dst = c;
               if (c == EOS)
                  break;
            }
            text[_BUFFER_SIZE - 1] = EOS;
         }
      }
      
      text_width = GetStringWidth(FONT_NORMAL, text, 0);
      x_offset   = ((s16)OPTION_VALUE_COLUMN_WIDTH - text_width) / 2 + OPTION_VALUE_COLUMN_X;
      //
      AddTextPrinterParameterized3(
         WIN_OPTIONS,
         FONT_NORMAL,
         x_offset,       // x
         (row * 16) + 1, // y
         sTextColor_OptionValues,
         TEXT_SKIP_DRAW,
         text
      );
      
      #undef _BUFFER_SIZE
   }
   
   if (is_single_update) {
      //CopyWindowRectToVram(WIN_OPTIONS, COPYWIN_GFX, 0, (row * 16) + 1, sOptionMenuWinTemplates[WIN_OPTIONS].width, 16);
      CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
   }
}

static void GetScrollInformation(u8* pos, u8* count) {
   u8 l_count;
   u8 l_scroll = sMenuState->cursor_pos;
   
   l_count = GetMenuItemListCount(GetCurrentMenuItemList());
   *count = l_count;
   
   if (l_scroll <= MENU_ITEM_HALFWAY_ROW || l_count <= MAX_MENU_ITEMS_VISIBLE_AT_ONCE) {
      l_scroll = 0;
   } else {
      l_scroll -= MENU_ITEM_HALFWAY_ROW;
      if (l_scroll + MAX_MENU_ITEMS_VISIBLE_AT_ONCE > l_count) {
         l_scroll = l_count - MAX_MENU_ITEMS_VISIBLE_AT_ONCE;
      }
   }
   *pos = l_scroll;
}
static u8 GetScrollPosition() {
   u8 count;
   u8 scroll;
   GetScrollInformation(&scroll, &count);
   return scroll;
}
static u8 GetScreenRowForCursorPos(void) {
   u8 count;
   u8 scroll;
   u8 pos = sMenuState->cursor_pos;
   
   count = GetMenuItemListCount(GetCurrentMenuItemList());
   if (count < MAX_MENU_ITEMS_VISIBLE_AT_ONCE) {
      return pos;
   }
   if (pos <= MENU_ITEM_HALFWAY_ROW) {
      return pos;
   }
   scroll = pos - MENU_ITEM_HALFWAY_ROW;
   if (scroll + MAX_MENU_ITEMS_VISIBLE_AT_ONCE > count) {
      scroll = count - MAX_MENU_ITEMS_VISIBLE_AT_ONCE;
   }
   return pos - scroll;
}
static void UpdateDisplayedMenuItems(void) {
   u8 i;
   u8 count;
   u8 scroll;
   const struct CGOptionMenuItem* items;
   
   items  = GetCurrentMenuItemList();
   count  = GetMenuItemListCount(items);
   scroll = GetScrollPosition();
   
   FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
   
   for(i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; ++i) {
      if (i + scroll >= count) {
         break;
      }
      DrawMenuItem(&items[i + scroll], i, FALSE);
   }
   
   RepaintScrollbar();
   
   CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
}
static void RepaintScrollbar(void) {
   u8 count;
   u8 scroll;
   GetScrollInformation(&scroll, &count);
   
   struct LuScrollbar* scrollbar = &sMenuState->widgets.scrollbar;
   scrollbar->scroll_pos = scroll;
   scrollbar->item_count = count;
   
   RepaintScrollbarV(scrollbar);
}
static void UpdateDisplayedControls(void) {
   u8 enabled_entries = 0;
   if (sMenuState->menu_view == MENUVIEW_IN_HELP) {
      enabled_entries |= 1 << KEYBIND_INDEX_RETURN_TO_MENU;
   } else {
      enabled_entries |= 1 << KEYBIND_INDEX_PICK;
      enabled_entries |= 1 << KEYBIND_INDEX_BACK;
      
      const struct CGOptionMenuItem* item;
      const struct CGOptionMenuItem* items = GetCurrentMenuItemList();
      
      item = NULL;
      if (items != NULL)
         item = &items[sMenuState->cursor_pos];
      
      if (item) {
         if (
            (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) != 0 ||
            item->value_type == VALUE_TYPE_POKEMON_SPECIES
         ) {
            enabled_entries |= 1 << KEYBIND_INDEX_ENTER_SUBMENU;
         } else {
            enabled_entries |= 1 << KEYBIND_INDEX_CHANGE;
            if (MenuItemTypeIsIntegral(item->value_type)) {
               enabled_entries |= 1 << KEYBIND_INDEX_TYPE_IN;
            }
         }
         if (GetRelevantHelpText(item) != NULL) {
            enabled_entries |= 1 << KEYBIND_INDEX_HELP;
         }
      }
   }
   sMenuState->widgets.keybind_strip.enabled_entries = enabled_entries;
   RepaintKeybindStrip(&sMenuState->widgets.keybind_strip);
}