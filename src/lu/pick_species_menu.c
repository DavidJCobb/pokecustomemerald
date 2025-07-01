#include "global.h"
#include "lu/pick_species_menu.h"

#include "battle_main.h" // gTypeNames
#include "bg.h"
#include "data.h" // gSpeciesNames
#include "decompress.h" // LoadCompressedSpriteSheet
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h" // AllocZeroed, Free
#include "menu.h"
#include "palette.h"
#include "pokemon.h" // gSpeciesInfo
#include "pokemon_icon.h"
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
#include "constants/pokemon.h" // TYPE_..., NUMBER_OF_MON_TYPES
#include "constants/species.h"
#include "lu/widgets/enum_picker.h"
#include "lu/widgets/keybind_strip.h"
#include "lu/widgets/loading_spinner.h"
#include "lu/widgets/scrollbar_v.h"
#include "lu/string_wrap.h"
#include "lu/strings.h"
#include "lu/ui_helpers.h"
#include "lu/vram_layout_helpers.h"

#include "lu/pick_species_menu.strings.h"
#include "lu/pokemon_icon_pool.h"
#include "lu/species_list_utils.h"

#include "gba/isagbprint.h"

enum {
   LISTING_POS_FIRST_LETTER,
   LISTING_POS_TYPE_1,
   LISTING_POS_TYPE_2,
   LISTING_POS_GENERATION,
   LISTING_POS_SWITCH_FOCUS_TO_RESULTS,
   FORM_ITEM_COUNT,
};

enum PaneEnum {
   PANE_FORM,
   PANE_LISTING,
};

enum {
   FIRST_LETTER_ANY = 0,
   FIRST_LETTER_A   = 1,
   FIRST_LETTER_Z   = 26,
   FIRST_LETTER_COUNT
};
enum {
   TYPE_CRITERION_ANY  = NUMBER_OF_MON_TYPES,
   TYPE_CRITERION_NONE = NUMBER_OF_MON_TYPES + 1,
   TYPE_CRITERION_VALUE_COUNT
};
enum {
   GENERATION_ANY = 0,
   GENERATION_KANTO,
   GENERATION_JOHTO,
   GENERATION_HOENN,
   
   GENERATIONS_COUNT
};

enum {
   SPRITE_PALETTE_TAG_ENUMPICKER = 4096,
   SPRITE_PALETTE_TAG_SPINNER    = 4097,
};

#define MAX_MON_ICONS 10

struct MenuState {
   struct PickSpeciesMenuParams params;
   
   u8  cursor_is_in_results;
   u16 cursor_pos;
   u16 selection;
   struct {
      u8 first_letter;
      u8 types[2];
      u8 generation;
   } filter_params;
   struct {
      struct SpeciesList contents;
      u16 scroll_pos;
      u8 sort_task;
      u8 anim_task;
      struct PokemonIconPool pokemon_icons;
   } listing;
   
   struct {
      struct LuEnumPicker   enum_picker;   // for filtering options
      struct LuKeybindStrip keybind_strip;
      struct LuScrollbar    scrollbar;     // for results listing
      
      u8 loading_spinner_sprite_id;
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
static const u8 sTextColor_OptionValuesDisabled[] = {1, 6, 7};

static const u16 sLoadingSpinnerPalette[] = INCBIN_U16("graphics/lu/spinner/spinner-32-aa-orange-on-white.gbapal");

enum {
   WIN_HEADER,
   WIN_FORM,
   WIN_LISTING,
   
   WIN_COUNT
};

#define SCROLLBAR_PALETTE_INDEX_BLANK 1
#define SCROLLBAR_PALETTE_INDEX_TRACK 14
#define SCROLLBAR_PALETTE_INDEX_THUMB 15

#define SELECTION_CURSOR_TILE_W 2
#define SELECTION_CURSOR_TILE_H 2

#define BACKGROUND_LAYER_NORMAL 0

#define BACKGROUND_PALETTE_ID_MENU 0
#define BACKGROUND_PALETTE_ID_TEXT 1
#define BACKGROUND_PALETTE_ID_CONTROLS 2
#define BACKGROUND_PALETTE_BOX_FRAME   7

#define TEXT_ROW_HEIGHT_IN_TILES   2

#define WIN_HEADER_TILE_WIDTH    DISPLAY_TILE_WIDTH
#define WIN_HEADER_TILE_HEIGHT   TEXT_ROW_HEIGHT_IN_TILES

#define WIN_FORM_TILE_X      SELECTION_CURSOR_TILE_W
#define WIN_FORM_TILE_Y      (WIN_HEADER_TILE_HEIGHT + 1)
#define WIN_FORM_TILE_WIDTH  12
#define WIN_FORM_TILE_HEIGHT (DISPLAY_TILE_HEIGHT - WIN_FORM_TILE_Y - KEYBIND_STRIP_TILE_HEIGHT)

#define FORM_CRITERION_VALUE_X 40 // relative to containing window
#define FORM_CRITERION_VALUE_W 50

#define WIN_LISTING_CURSOR_X    (WIN_FORM_TILE_X + WIN_FORM_TILE_WIDTH)
#define WIN_LISTING_ICONS_X     ((WIN_LISTING_CURSOR_X + SELECTION_CURSOR_TILE_W) * TILE_WIDTH)

#define WIN_LISTING_TILE_X      ((WIN_LISTING_ICONS_X / TILE_WIDTH) + 3)
#define WIN_LISTING_TILE_Y      WIN_FORM_TILE_Y
#define WIN_LISTING_TILE_WIDTH  (DISPLAY_TILE_WIDTH - WIN_LISTING_TILE_X - 1)
#define WIN_LISTING_TILE_HEIGHT WIN_FORM_TILE_HEIGHT

#define MAX_MENU_ITEMS_VISIBLE_AT_ONCE   (WIN_LISTING_TILE_HEIGHT / TEXT_ROW_HEIGHT_IN_TILES)
#define MENU_ITEM_HALFWAY_ROW            (MAX_MENU_ITEMS_VISIBLE_AT_ONCE / 2)

// Never instantiated. Just a marginally less hideous way to manage all this 
// compared to preprocessor macros. Unit of measurement is 4bpp tile IDs.
typedef struct {
   VRAMTile transparent_tile;
   VRAMTile blank_tile;
   VRAMTile selection_cursor_tiles[SELECTION_CURSOR_TILE_W * SELECTION_CURSOR_TILE_H];
   VRAMTile user_window_frame[9];
   VRAMTile scrollbar_tiles[SCROLLBAR_TILE_COUNT];
   
   VRAMTilemap tilemaps[4];
   
   VRAMTile win_tiles_for_header[WIN_HEADER_TILE_WIDTH * WIN_HEADER_TILE_HEIGHT];
   VRAMTile win_tiles_for_keybinds[KEYBIND_STRIP_TILE_COUNT];
   VRAMTile win_tiles_for_form[WIN_FORM_TILE_WIDTH * WIN_FORM_TILE_HEIGHT];
   VRAMTile win_tiles_for_listing[WIN_LISTING_TILE_WIDTH * WIN_LISTING_TILE_HEIGHT];
} VRAMTileLayout;
STATIC_ASSERT(sizeof(VRAMTileLayout) <= BG_VRAM_SIZE, sStaticAssertion01_VramUsage);

static const struct LuEnumPickerInitParams sEnumPickerInit = {
   .base_pos = {
      .x = (WIN_FORM_TILE_X * TILE_WIDTH) + FORM_CRITERION_VALUE_X,
      .y = (WIN_LISTING_TILE_Y * TILE_HEIGHT),
   },
   .sprite_tags = {
      .tile    = 4096,
      .palette = SPRITE_PALETTE_TAG_ENUMPICKER,
   },
   .width = FORM_CRITERION_VALUE_W,
};
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

//

static void Task_MenuFadeIn(u8 taskId);
static void Task_MenuProcessInput(u8 taskId);
static void Task_MenuFadeOut(u8 taskId);

static void PaintMenuHeader(void);
static void PaintForm(void);
static void PaintCursor(void);
static void PaintListingContents(void);
static void UpdateKeybindStrip(void);

static void MoveCursor(s8 by);

static u16 GetListingScrollPosition(void);

static void SetFocusedPane(enum PaneEnum);

static bool8 IsAsyncSortInProgress(void);
static void OnFilterChanged(void);

static void ChildTask_AnimatePokemonIcons(u8 taskId); // subordinate task

static void CB2_InitPickSpeciesMenu(void) {
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
         
         SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
         
         gMain.state++;
         break;
       case 2:
         LuUI_ResetSpritesAndEffects();
         //ResetTasks(); // we're called from a task-based menu
         
         VRAM_BG_LoadTiles(VRAMTileLayout, blank_tile, BACKGROUND_LAYER_NORMAL, sBlankBGTile);
         VRAM_BG_LoadTiles(VRAMTileLayout, selection_cursor_tiles, BACKGROUND_LAYER_NORMAL, sMenuCursorBGTiles);
         
         gMain.state++;
         break;
       case 3:
         LuUI_LoadPlayerWindowFrame(
            BACKGROUND_LAYER_NORMAL,
            BACKGROUND_PALETTE_BOX_FRAME,
            VRAM_BG_TileID(VRAMTileLayout, user_window_frame)
         );
         gMain.state++;
         break;
       case 4:
         LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(BACKGROUND_PALETTE_ID_MENU), sizeof(sOptionMenuBg_Pal));
         gMain.state++;
         break;
       case 5:
         LoadPalette(sOptionsListingPalette, BG_PLTT_ID(BACKGROUND_PALETTE_ID_TEXT), sizeof(sOptionsListingPalette));
         LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(BACKGROUND_PALETTE_ID_CONTROLS), PLTT_SIZE_4BPP);
         gMain.state++;
         break;
       case 6:
         PutWindowTilemap(WIN_HEADER);
         gMain.state++;
         break;
       case 7:
         PutWindowTilemap(WIN_FORM);
         gMain.state++;
         break;
       case 8:
         PutWindowTilemap(WIN_LISTING);
         FillWindowPixelBuffer(WIN_LISTING, PIXEL_FILL(1));
         CopyWindowToVram(WIN_LISTING, COPYWIN_FULL);
         gMain.state++;
         break;
       case 9:
         //
         gMain.state++;
         break;
       case 10:
         {
            u8 taskId = CreateTask(Task_MenuFadeIn, 0);
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
            {
               sMenuState->widgets.loading_spinner_sprite_id = SpawnLoadingSpinner(
                  (WIN_LISTING_TILE_X + WIN_LISTING_TILE_WIDTH  / 2) * TILE_WIDTH,
                  (WIN_LISTING_TILE_Y + WIN_LISTING_TILE_HEIGHT / 2) * TILE_HEIGHT,
                  4097,
                  SPRITE_PALETTE_TAG_SPINNER,
                  sLoadingSpinnerPalette,
                  0,
                  FALSE
               );
            }
            PaintMenuHeader();
            PaintForm();
            SetFocusedPane(PANE_FORM);
            OnFilterChanged();

            CopyWindowToVram(WIN_FORM, COPYWIN_FULL);
            CopyWindowToVram(WIN_LISTING, COPYWIN_FULL);
            gMain.state++;
         }
         break;
       case 11:
         BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
         SetVBlankCallback(VBlankCB);
         SetMainCallback2(MainCB2);
         return;
   }
}
extern void ShowPickSpeciesMenu(const struct PickSpeciesMenuParams* params) {
   sMenuState = AllocZeroed(sizeof(struct MenuState));
   sMenuState->params = *params;
   sMenuState->selection = PICKSPECIESMENU_RESULT_CANCELED;
   
   sMenuState->filter_params.types[0] = TYPE_CRITERION_ANY;
   sMenuState->filter_params.types[1] = TYPE_CRITERION_ANY;
   
   sMenuState->listing.sort_task = TASK_NONE;
   sMenuState->listing.anim_task = CreateTask(ChildTask_AnimatePokemonIcons, 1);
   
   InitPokemonIconPool(&sMenuState->listing.pokemon_icons, MAX_MON_ICONS);
   
   SetMainCallback2(CB2_InitPickSpeciesMenu);
}

static void Task_MenuFadeIn(u8 taskId) {
   if (!gPaletteFade.active)
      gTasks[taskId].func = Task_MenuProcessInput;
}

static void Task_MenuProcessInput(u8 taskId) {
   {
      s8 by = 0;
      if (sMenuState->cursor_is_in_results) {
         if (JOY_REPEAT(DPAD_UP))
            by = -1;
         else if (JOY_REPEAT(DPAD_DOWN))
            by = 1;
      } else {
         if (JOY_NEW(DPAD_UP))
            by = -1;
         else if (JOY_NEW(DPAD_DOWN))
            by = 1;
      }
      if (by)
         MoveCursor(by);
   }
   if (sMenuState->cursor_is_in_results) {
      if (JOY_NEW(A_BUTTON)) {
         if (sMenuState->listing.contents.count > 0) {
            sMenuState->selection = sMenuState->listing.contents.speciesIDs[sMenuState->cursor_pos];
         }
         gTasks[taskId].func = Task_MenuFadeOut;
         return;
      }
      if (JOY_NEW(B_BUTTON)) {
         SetFocusedPane(PANE_FORM);
         return;
      }
   } else {
      if (JOY_NEW(A_BUTTON)) {
         if (sMenuState->cursor_pos == LISTING_POS_SWITCH_FOCUS_TO_RESULTS) {
            if (IsAsyncSortInProgress()) {
               PlaySE(SE_FAILURE);
               return;
            }
            SetFocusedPane(PANE_LISTING);
         }
         return;
      }
      if (JOY_NEW(B_BUTTON)) {
         gTasks[taskId].func = Task_MenuFadeOut;
         return;
      }
      //
      // Handle option changes:
      //
      s8 by = JOY_REPEAT(DPAD_RIGHT) ? 1 : 0;
      if (!by) {
         by = JOY_REPEAT(DPAD_LEFT) ? -1 : 0;
      }
      if (by) {
         {
            u8* value = NULL;
            u8  max   = 0;
            switch (sMenuState->cursor_pos) {
               case LISTING_POS_TYPE_1:
               case LISTING_POS_TYPE_2:
                  {
                     bool8 is_second = (sMenuState->cursor_pos == LISTING_POS_TYPE_2);
                     u8*   target    = &sMenuState->filter_params.types[is_second ? 1 : 0];
                     if (by < 0) {
                        if (*target == 0) {
                           *target = TYPE_CRITERION_VALUE_COUNT - 1;
                        } else {
                           --(*target);
                           if (*target == TYPE_MYSTERY) {
                              --(*target);
                           }
                        }
                        if (!is_second && *target == TYPE_CRITERION_NONE) {
                           *target = TYPE_CRITERION_ANY;
                        }
                     } else {
                        ++(*target);
                        if (*target == TYPE_MYSTERY) {
                           ++(*target);
                        }
                        if (*target == TYPE_CRITERION_VALUE_COUNT) {
                           *target = 0;
                        }
                        if (!is_second && *target == TYPE_CRITERION_NONE) {
                           *target = 0;
                        }
                     }
                  }
                  break;
               case LISTING_POS_FIRST_LETTER:
                  value = &sMenuState->filter_params.first_letter;
                  max   = FIRST_LETTER_Z;
                  break;
               case LISTING_POS_GENERATION:
                  value = &sMenuState->filter_params.generation;
                  max   = GENERATIONS_COUNT - 1;
                  break;
            }
            if (value) {
               if (by < 0) {
                  if (*value == 0)
                     *value = max;
                  else
                     --(*value);
               } else {
                  ++(*value);
                  if (*value > max)
                     *value = 0;
               }
            }
         }
         OnFilterChanged();
         PaintForm();
         if (by < 0) {
            OnEnumPickerDecreased(&sMenuState->widgets.enum_picker);
         } else {
            OnEnumPickerIncreased(&sMenuState->widgets.enum_picker);
         }
         return;
      }
   }
}

static void Task_MenuFadeOut(u8 taskId) {
   if (gPaletteFade.active)
      return;
   
   void (*callback)(u16) = sMenuState->params.callback;
   u16 value = sMenuState->selection;
   
   if (sMenuState->listing.sort_task != TASK_NONE) {
      DestroyTask(sMenuState->listing.sort_task);
      sMenuState->listing.sort_task = TASK_NONE;
   }
   if (sMenuState->listing.anim_task != TASK_NONE) {
      DestroyTask(sMenuState->listing.anim_task);
      sMenuState->listing.anim_task = TASK_NONE;
   }
   DestroyTask(taskId);
   DestroyEnumPicker(&sMenuState->widgets.enum_picker);
   DestroyKeybindStrip(&sMenuState->widgets.keybind_strip);
   DestroyScrollbarV(&sMenuState->widgets.scrollbar);
   DestroyLoadingSpinner(sMenuState->widgets.loading_spinner_sprite_id);
   DestroyPokemonIconPool(&sMenuState->listing.pokemon_icons);
   FreeAllWindowBuffers();
   Free(sMenuState);
   sMenuState = NULL;
   
   callback(value);
}

static void PaintMenuHeader(void) {
   const u8* title = sMenuState->params.header_text;
   if (title == NULL) {
      title = gText_lu_PickSpeciesMenu_DefaultTitle;
   }
   FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
   AddTextPrinterParameterized(WIN_HEADER, FONT_BOLD, title, 8, 1, TEXT_SKIP_DRAW, NULL);
   CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void PaintForm(void) {
   u8 dynamic_text_buffer[5];
   
   FillWindowPixelBuffer(WIN_FORM, PIXEL_FILL(1));
   
   for(u8 i = 0; i < FORM_ITEM_COUNT; ++i) {
      bool8     disabled   = FALSE;
      bool8     hyperlink  = FALSE;
      const u8* label_text = NULL;
      const u8* value_text = NULL;
      switch (i) {
         case LISTING_POS_FIRST_LETTER:
            label_text = gText_lu_PickSpeciesMenu_Label_FirstLetter;
            {
               u8 value = sMenuState->filter_params.first_letter;
               if (value == 0) {
                  value_text = gText_lu_PickSpeciesMenu_FirstLetter_Any;
               } else {
                  dynamic_text_buffer[0] = CHAR_A + value - FIRST_LETTER_A;
                  dynamic_text_buffer[1] = EOS;
                  value_text = dynamic_text_buffer;
               }
            }
            break;
         case LISTING_POS_TYPE_1:
         case LISTING_POS_TYPE_2:
            label_text = gText_lu_PickSpeciesMenu_Label_Type1;
            {
               u8 value = sMenuState->filter_params.types[0];
               if (i == LISTING_POS_TYPE_2) {
                  value      = sMenuState->filter_params.types[1];
                  label_text = gText_lu_PickSpeciesMenu_Label_Type2;
               }
               if (value == TYPE_CRITERION_ANY) {
                  value_text = gText_lu_PickSpeciesMenu_Type_Any;
               } else if (value == TYPE_CRITERION_NONE) {
                  value_text = gText_lu_PickSpeciesMenu_Type_None;
               } else {
                  value_text = gTypeNames[value];
               }
            }
            break;
         case LISTING_POS_GENERATION:
            label_text = gText_lu_PickSpeciesMenu_Label_Generation;
            {
               value_text = gText_lu_PickSpeciesMenu_Generation_Any;
               switch (sMenuState->filter_params.generation) {
                  case GENERATION_KANTO:
                     value_text = gText_lu_PickSpeciesMenu_Generation_Kanto;
                     break;
                  case GENERATION_JOHTO:
                     value_text = gText_lu_PickSpeciesMenu_Generation_Johto;
                     break;
                  case GENERATION_HOENN:
                     value_text = gText_lu_PickSpeciesMenu_Generation_Hoenn;
                     break;
               }
            }
            break;
         case LISTING_POS_SWITCH_FOCUS_TO_RESULTS:
            label_text = gText_lu_PickSpeciesMenu_Label_SwitchToListing;
            hyperlink  = TRUE;
            disabled   = IsAsyncSortInProgress();
            break;
      }
      
      if (label_text) {
         const u8* colors = sTextColor_OptionNames;
         if (hyperlink) {
            colors = sTextColor_OptionValues;
            if (disabled) {
               colors = sTextColor_OptionValuesDisabled;
            }
         }
         AddTextPrinterParameterized3(
            WIN_FORM,
            FONT_NORMAL,
            0, // x
            (i * (TILE_HEIGHT * 2)), // y
            colors,
            TEXT_SKIP_DRAW,
            label_text
         );
      }
      if (value_text) {
         const u8* colors = sTextColor_OptionValues;
         if (disabled) {
            colors = sTextColor_OptionValuesDisabled;
         }
         
         u8 w = GetStringWidth(FONT_NORMAL, value_text, 0);
         u8 x = FORM_CRITERION_VALUE_X;
         if (w < FORM_CRITERION_VALUE_X) {
            x += (FORM_CRITERION_VALUE_W - w) / 2;
         }
         
         AddTextPrinterParameterized3(
            WIN_FORM,
            FONT_NORMAL,
            x, // x
            (i * (TILE_HEIGHT * TEXT_ROW_HEIGHT_IN_TILES)), // y
            colors,
            TEXT_SKIP_DRAW,
            value_text
         );
      }
   }
   CopyWindowToVram(WIN_FORM, COPYWIN_GFX);
}

static void PaintCursor(void) {
   FillBgTilemapBufferRect(
      BACKGROUND_LAYER_NORMAL,
      VRAM_BG_TileID(VRAMTileLayout, blank_tile),
      WIN_FORM_TILE_X - SELECTION_CURSOR_TILE_W,
      WIN_FORM_TILE_Y,
      SELECTION_CURSOR_TILE_W, // width to paint over
      WIN_FORM_TILE_HEIGHT,
      BACKGROUND_PALETTE_ID_TEXT
   );
   FillBgTilemapBufferRect(
      BACKGROUND_LAYER_NORMAL,
      VRAM_BG_TileID(VRAMTileLayout, blank_tile),
      WIN_LISTING_CURSOR_X,
      WIN_LISTING_TILE_Y,
      SELECTION_CURSOR_TILE_W, // width to paint over
      WIN_LISTING_TILE_HEIGHT,
      BACKGROUND_PALETTE_ID_TEXT
   );
   
   u8 base_x;
   if (sMenuState->cursor_is_in_results) {
      base_x = WIN_LISTING_CURSOR_X;
   } else {
      base_x = 0;
   }
   u16 pos = sMenuState->cursor_pos - GetListingScrollPosition();
   {
      u8 i;
      u8 x;
      u8 y;
      for(i = 0; i < VRAM_BG_TileCount(VRAMTileLayout, selection_cursor_tiles); ++i) {
         x = i % SELECTION_CURSOR_TILE_W;
         y = i / SELECTION_CURSOR_TILE_W;
         FillBgTilemapBufferRect(
            BACKGROUND_LAYER_NORMAL,
            VRAM_BG_TileID(VRAMTileLayout, selection_cursor_tiles[0]) + i,
            base_x + x,
            WIN_FORM_TILE_Y + (pos * TEXT_ROW_HEIGHT_IN_TILES) + y,
            1,
            1,
            BACKGROUND_PALETTE_ID_TEXT
         );
      }
   }
   CopyBgTilemapBufferToVram(BACKGROUND_LAYER_NORMAL);
}
static void PaintListingItem(u8 display_pos, PokemonSpeciesID species) {
   const u8* text = gSpeciesNames[species];
   if (species == 0) {
      if (sMenuState->params.zero_type == PICKSPECIESMENU_ZEROTYPE_DEFAULT) {
         text = gText_lu_PickSpeciesMenu_Species_Default;
      } else if (sMenuState->params.zero_type == PICKSPECIESMENU_ZEROTYPE_NONE) {
         text = gText_lu_PickSpeciesMenu_Species_None;
      }
   }
   AddTextPrinterParameterized3(
      WIN_LISTING,
      FONT_NORMAL,
      0, // x
      (display_pos * (TILE_HEIGHT * TEXT_ROW_HEIGHT_IN_TILES)) + 1, // y
      sTextColor_OptionNames,
      TEXT_SKIP_DRAW,
      text
   );
}
static void PaintListingContents(void) {
   FillBgTilemapBufferRect(
      BACKGROUND_LAYER_NORMAL,
      VRAM_BG_TileID(VRAMTileLayout, blank_tile),
      (WIN_LISTING_ICONS_X / TILE_WIDTH),
      WIN_LISTING_TILE_Y,
      3, // width to paint over
      WIN_LISTING_TILE_HEIGHT,
      BACKGROUND_PALETTE_ID_TEXT
   );
   if (IsAsyncSortInProgress()) {
      SetLoadingSpinnerVisible(sMenuState->widgets.loading_spinner_sprite_id, TRUE);
      
      struct LuScrollbar* scrollbar = &sMenuState->widgets.scrollbar;
      scrollbar->scroll_pos = 0;
      scrollbar->item_count = 1;
      RepaintScrollbarV(scrollbar);
      
      FillWindowPixelBuffer(WIN_LISTING, PIXEL_FILL(1));
      CopyWindowToVram(WIN_LISTING, COPYWIN_GFX);
      
      SetPooledPokemonIconsVisible(&sMenuState->listing.pokemon_icons, FALSE);
      
      return;
   }
   
   SetLoadingSpinnerVisible(sMenuState->widgets.loading_spinner_sprite_id, FALSE);
   
   u16 pos   = GetListingScrollPosition();
   u16 count = sMenuState->listing.contents.count;
   {
      struct LuScrollbar* scrollbar = &sMenuState->widgets.scrollbar;
      scrollbar->scroll_pos = pos;
      scrollbar->item_count = count;
      RepaintScrollbarV(scrollbar);
   }
   
   FillWindowPixelBuffer(WIN_LISTING, PIXEL_FILL(1));
   for(int i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; ++i) {
      if (i + pos >= count)
         break;
      PaintListingItem(i, sMenuState->listing.contents.speciesIDs[i + pos]);
   }
   CopyWindowToVram(WIN_LISTING, COPYWIN_GFX);
   
   const u8 Y_OFFSET = 4;
   //
   // Update sprites.
   //
   {
      struct PokemonIconPool* pool = &sMenuState->listing.pokemon_icons;
      SetPooledPokemonIconsVisible(pool, TRUE);
      MarkAllPooledPokemonIconsForDelete(pool);
      for(int i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; ++i) {
         if (i + pos >= count)
            break;
         PokemonSpeciesID species = sMenuState->listing.contents.speciesIDs[i + pos];
         
         u16 j = FindPooledPokemonIconBySpecies(pool, species);
         if (j != NO_POOLED_POKEMON_ICON) {
            UnmarkPooledPokemonIconForDelete(pool, j);
            
            u8 y = Y_OFFSET + (WIN_LISTING_TILE_Y * TILE_HEIGHT) + i * (TILE_HEIGHT * TEXT_ROW_HEIGHT_IN_TILES);
            SetPooledPokemonIconPosition(pool, j, WIN_LISTING_ICONS_X + 8, y); // sprite X-coordinates are centered
         }
      }
      DeleteMarkedPooledPokemonIcons(pool);
      for(int i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; ++i) {
         if (i + pos >= count)
            break;
         PokemonSpeciesID species = sMenuState->listing.contents.speciesIDs[i + pos];
         
         u16 j = FindPooledPokemonIconBySpecies(pool, species);
         if (j != NO_POOLED_POKEMON_ICON) {
            continue;
         }
         
         u8 y = Y_OFFSET + (WIN_LISTING_TILE_Y * TILE_HEIGHT) + i * (TILE_HEIGHT * TEXT_ROW_HEIGHT_IN_TILES);
         AddPooledPokemonIcon(pool, species, WIN_LISTING_ICONS_X + 8, y); // sprite X-coordinates are centered
      }
   }
}
static void UpdateKeybindStrip(void) {
   u8 enabled_entries = (1 << 0) | (1 << 4);
   if (sMenuState->cursor_is_in_results) {
      enabled_entries |= 1 << 3; // Choose Species
   } else {
      if (sMenuState->cursor_pos == LISTING_POS_SWITCH_FOCUS_TO_RESULTS) {
         enabled_entries |= 1 << 2; // View Search Results
      } else {
         enabled_entries |= 1 << 1; // Change
      }
   }
   sMenuState->widgets.keybind_strip.enabled_entries = enabled_entries;
   RepaintKeybindStrip(&sMenuState->widgets.keybind_strip);
}

static void MoveCursor(s8 by) {
   if (by == 0) {
      return;
   }
   
   u16 max;
   if (sMenuState->cursor_is_in_results) {
      max = sMenuState->listing.contents.count;
      if (max > 0)
         --max;
   } else {
      max = LISTING_POS_SWITCH_FOCUS_TO_RESULTS;
   }
   
   if (by < 0) {
      if (sMenuState->cursor_pos == 0) {
         sMenuState->cursor_pos = max;
      } else {
         --sMenuState->cursor_pos;
      }
   } else {
      if (sMenuState->cursor_pos == max) {
         sMenuState->cursor_pos = 0;
      } else {
         ++sMenuState->cursor_pos;
      }
   }
   if (sMenuState->cursor_is_in_results) {
      SetEnumPickerVisible(&sMenuState->widgets.enum_picker, FALSE);
   } else {
      if (sMenuState->cursor_pos == LISTING_POS_SWITCH_FOCUS_TO_RESULTS) {
         SetEnumPickerVisible(&sMenuState->widgets.enum_picker, FALSE);
      } else {
         SetEnumPickerVisible(&sMenuState->widgets.enum_picker, TRUE);
         SetEnumPickerRow(&sMenuState->widgets.enum_picker, sMenuState->cursor_pos);
      }
   }
   if (sMenuState->cursor_is_in_results) {
      PaintListingContents();
   } else {
      UpdateKeybindStrip();
   }
   PaintCursor();
   PlaySE(SE_SELECT);
}
static u16 GetListingScrollPosition(void) {
   u16 pos   = sMenuState->cursor_pos;
   u16 count = sMenuState->listing.contents.count;
   if (!sMenuState->cursor_is_in_results) {
      pos = 0;
   }
   if (pos <= MENU_ITEM_HALFWAY_ROW || count <= MAX_MENU_ITEMS_VISIBLE_AT_ONCE) {
      pos = 0;
   } else {
      pos -= MENU_ITEM_HALFWAY_ROW;
      if (pos + MAX_MENU_ITEMS_VISIBLE_AT_ONCE > count) {
         pos = count - MAX_MENU_ITEMS_VISIBLE_AT_ONCE;
      }
   }
   return pos;
}

static void SetFocusedPane(enum PaneEnum pane) {
   sMenuState->cursor_pos = 0;
   if (pane == PANE_LISTING) {
      sMenuState->cursor_is_in_results = TRUE;
      SetEnumPickerVisible(&sMenuState->widgets.enum_picker, FALSE);
   } else {
      sMenuState->cursor_is_in_results = FALSE;
      SetEnumPickerVisible(&sMenuState->widgets.enum_picker, TRUE);
      SetEnumPickerRow(&sMenuState->widgets.enum_picker, 0);
   }
   PaintCursor();
   UpdateKeybindStrip();
   //
   // TODO: indicate where focus is at (darken the unfocused pane?)
   //
}

//
// Sorting/filtering
//

static void OnAsyncSortSpeciesListDone(void);

static bool8 IsAsyncSortInProgress(void) {
   return sMenuState->listing.sort_task != TASK_NONE;
}

static bool8 FilterSpecies(PokemonSpeciesID species) {
   if (species == SPECIES_NONE) {
      return (sMenuState->params.zero_type != PICKSPECIESMENU_ZEROTYPE_DISALLOWED);
   }
   if (species >= SPECIES_OLD_UNOWN_B && species <= SPECIES_OLD_UNOWN_Z) {
      return FALSE;
   }
   if (sMenuState->filter_params.first_letter) {
      u8 ch = CHAR_A + sMenuState->filter_params.first_letter - FIRST_LETTER_A;
      if (gSpeciesNames[species][0] != ch) {
         return FALSE;
      }
   }
   {  // Types
      if (sMenuState->filter_params.types[0] != TYPE_CRITERION_ANY) {
         if (sMenuState->filter_params.types[1] == TYPE_CRITERION_ANY) {
            //
            // If the user has only picked one type (e.g. FLYING/ANY), then allow 
            // dual-type Pokemon to match if either of their types is the type 
            // the user picked.
            //
            // The edge-case we want to handle here is the fact that as of Gen III, 
            // no Pokemon has FLYING as its first type; every single one of them is 
            // dual-type. The player shouldn't have to select ANY/FLYING to view 
            // these Pokemon; this check makes FLYING/ANY sufficient.
            //
            if (
               gSpeciesInfo[species].types[0] != sMenuState->filter_params.types[0] &&
               gSpeciesInfo[species].types[1] != sMenuState->filter_params.types[0]
            ) {
               return FALSE;
            }
         } else {
            if (gSpeciesInfo[species].types[0] != sMenuState->filter_params.types[0])
               return FALSE;
         }
      }
      if (sMenuState->filter_params.types[1] == TYPE_CRITERION_NONE) {
         u8 type = sMenuState->filter_params.types[1];
         if (type != TYPE_NONE && type != sMenuState->filter_params.types[0]) {
            //
            // Monotypes in vanilla are encoded as e.g. [TYPE_WATER, TYPE_WATER] 
            // rather than [TYPE_WATER, TYPE_NONE] as you might expect.
            //
            return FALSE;
         }
      } else if (sMenuState->filter_params.types[1] != TYPE_CRITERION_ANY) {
         if (gSpeciesInfo[species].types[1] != sMenuState->filter_params.types[1]) {
            return FALSE;
         }
      }
   }
   {  // Generation
      u8 criterion = sMenuState->filter_params.generation;
      switch (criterion) {
         case GENERATION_KANTO:
            if (species > SPECIES_MEW)
               return FALSE;
            break;
         case GENERATION_JOHTO:
            if (species <= SPECIES_MEW)
               return FALSE;
            if (species > SPECIES_CELEBI)
               return FALSE;
            break;
         case GENERATION_HOENN:
            if (species <= SPECIES_TREECKO)
               return FALSE;
            if (species > SPECIES_CHIMECHO)
               return FALSE;
            break;
      }
   }
   return TRUE;
}
static bool8 SortSpecies(PokemonSpeciesID a, PokemonSpeciesID b) { // returns a < b

   // Always sort species ID 0 at the top.
   if (a == 0) {
      return TRUE;
   } else if (b == 0) {
      return FALSE;
   }
   
   // Alphabetical sort.
   for(int i = 0; i < POKEMON_NAME_LENGTH; ++i) {
      u8 x = gSpeciesNames[a][i];
      u8 y = gSpeciesNames[b][i];
      
      // Shorter names first.
      if (x == EOS) {
         return TRUE;
      } else if (y == EOS) {
         return FALSE;
      }
      
      if (x != y)
         return x < y;
   }
   //
   // Names are equal.
   //
   return TRUE;
}
static void OnFilterChanged(void) {
   if (sMenuState->listing.sort_task != TASK_NONE) {
      DestroyTask(sMenuState->listing.sort_task);
      sMenuState->listing.sort_task = TASK_NONE;
      DebugPrintf("[OnFilterChanged] Destroyed ongoing async sort.");
   }
   
   AllocFilteredSpeciesList(
      &sMenuState->listing.contents,
      FilterSpecies
   );
   DebugPrintf("[OnFilterChanged] Species list allocated.");
   
   if (sMenuState->listing.contents.count > 200) {
      sMenuState->listing.sort_task = SortSpeciesListAsync(
         &sMenuState->listing.contents,
         SortSpecies,
         600,
         OnAsyncSortSpeciesListDone
      );
      DebugPrintf("[OnFilterChanged] Species list is large. Created/restarted a task to sort it for us...");
      PaintListingContents();
   } else {
      SortSpeciesList(
         &sMenuState->listing.contents,
         SortSpecies
      );
      DebugPrintf("[OnFilterChanged] Species list sorted.");
      PaintListingContents();
      DebugPrintf("[OnFilterChanged] Species list painted.");
   }
}

static void OnAsyncSortSpeciesListDone(void) {
   DebugPrintf("Async sort complete.");
   sMenuState->listing.sort_task = TASK_NONE;
   PaintForm(); // un-grey-out the menu item to enter the listing
   PaintListingContents();
}

//
// Animating Pokemon icons
//

static void ChildTask_AnimatePokemonIcons(u8 taskId) {
   struct PokemonIconPool* pool = &sMenuState->listing.pokemon_icons;
   
   u8 focused_sprite = 0xFF;
   if (sMenuState->cursor_is_in_results) {
      focused_sprite = FindPooledPokemonIconBySpecies(
         pool,
         sMenuState->listing.contents.speciesIDs[sMenuState->cursor_pos]
      );
   }
   
   for(int i = 0; i < pool->count; ++i) {
      u8 id = pool->sprite_ids[i];
      if (id >= MAX_SPRITES)
         continue;
      struct Sprite* sprite = &gSprites[id];
      if (i == focused_sprite) {
         UpdateMonIconFrame(sprite);
         sprite->subpriority = 0;
      } else {
         //
         // Forcibly return the sprite to its first frame.
         //
         sprite->animDelayCounter = sprite->anims[sprite->animNum][0].frame.duration & 0xFF;
         if (sprite->animCmdIndex > 0) {
            sprite->animCmdIndex = 0;
            RequestSpriteCopy(
               (u8*)sprite->images,
               (u8*)(OBJ_VRAM0 + sprite->oam.tileNum * TILE_SIZE_4BPP),
               512
            );
            //
            sprite->subpriority = 2;
         }
      }
   }
}