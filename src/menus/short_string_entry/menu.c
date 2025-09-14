#include "menus/short_string_entry/menu.h"
#include "menus/short_string_entry/cursors.h"
#include "menus/short_string_entry/icon.h"
#include "menus/short_string_entry/state.h"
#include "menus/short_string_entry/fragments/charset_buttons.h"
#include "menus/short_string_entry/widget_grid.h"

#include "lu/vui/vui-context.h"
#include "lu/vui/vui-frame.h"
#include "lu/vui/custom-keyboard.h"
#include "lu/vui/keyboard.h"
#include "lu/vui/keyboard-value.h"
#include "lu/vui/sprite-button.h"
#include "lu/vui/tile-button.h"

#include "gba/gba.h"
#include "gba/isagbprint.h"
#include "bg.h"
#include "decompress.h" // gDecompressionBuffer
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h" // AddTextPrinterParameterized3
#include "palette.h"
#include "sound.h" // PlaySE
#include "string_util.h"
#include "strings.h" // gText_FemaleSymbol, gText_MaleSymbol
#include "task.h"
#include "text.h"
#include "text_window.h" // GetTextWindowPalette
#include "window.h"
#include "constants/characters.h"
#include "constants/pokemon.h" // MON_MALE, MON_FEMALE, MON_GENDERLESS
#include "constants/rgb.h"
#include "constants/songs.h" // SE_SELECT and other sound effect constants
#include "lu/c.h"
#include "lu/gfxutils.h" // BlitBitmapRect4BitRemapped
#include "lu/ui_helpers.h"
#include "lu/vram_layout_helpers_new.h"
//
extern const u8 gSpeciesNames[][POKEMON_NAME_LENGTH + 1]; // from `data.h`

static const u8  sBGTileGfx[]   = INCBIN_U8("graphics/lu/short_string_entry_menu/bg-tiles.4bpp");
static const u32 sBGTilemap[]   = INCBIN_U32("graphics/lu/short_string_entry_menu/bg-tiles.bin");
static const u16 sBGPalette[16] = INCBIN_U16("graphics/lu/short_string_entry_menu/bg.gbapal");

static const u8 sBGGenderTiles[] = INCBIN_U8("graphics/lu/short_string_entry_menu/bg-gender-icon-fragment.4bpp");

static const u8  sMenuButtonFaceGfx[]   = INCBIN_U8("graphics/lu/short_string_entry_menu/menu-button-face-gfx.4bpp");
static const u16 sMenuButtonFacePal[16] = INCBIN_U16("graphics/lu/short_string_entry_menu/menu-button-face-pal.gbapal");

static const u8 sBlankBGTile[] = INCBIN_U8("graphics/lu/cgo_menu/bg-tile-blank.4bpp"); // color 1

enum {
   BGLAYER_BACKDROP = 0,
   BGLAYER_BUTTONS  = 1,
   BGLAYER_TEXT     = 2,
};
enum {
   PALETTE_ID_BACKDROP   =  0,
   PALETTE_ID_MENUBUTTON =  1,
   PALETTE_ID_USER_BORDER = 2,
   PALETTE_ID_TEXT       = 15,
};
enum {
   VALUE_TILE_X = 8,
   VALUE_TILE_Y = 3,
   
   WIN_BTN_OK_X = 192,
   WIN_BTN_OK_Y =  56,
   WIN_BTN_OK_W =  24,
   WIN_BTN_OK_H =  32,
   WIN_BTN_OK_TILE_COUNT = (WIN_BTN_OK_W / TILE_WIDTH) * (WIN_BTN_OK_H / TILE_HEIGHT),
   
   WIN_BTN_BKSP_X = WIN_BTN_OK_X,
   WIN_BTN_BKSP_Y = 96,
   WIN_BTN_BKSP_W = WIN_BTN_OK_W,
   WIN_BTN_BKSP_H = WIN_BTN_OK_H,
   WIN_BTN_BKSP_TILE_COUNT = (WIN_BTN_BKSP_W / TILE_WIDTH) * (WIN_BTN_BKSP_H / TILE_HEIGHT),
   
   KEYBOARD_TILE_X       = 7,
   KEYBOARD_TILE_Y       = 7,
   KEYBOARD_TILE_INNER_W = VUIKEYBOARD_INNER_W_TILES,
   KEYBOARD_TILE_INNER_H = VUIKEYBOARD_INNER_H_TILES,
   
   TITLE_WINDOW_TILE_X      = 6,
   TITLE_WINDOW_TILE_WIDTH  = DISPLAY_TILE_WIDTH - TITLE_WINDOW_TILE_X - 1,
   TITLE_WINDOW_TILE_HEIGHT = 2,
   TITLE_WINDOW_TILE_COUNT  = TITLE_WINDOW_TILE_WIDTH * TITLE_WINDOW_TILE_HEIGHT,
   
   GENDER_WINDOW_TEXT_X = 32,
   GENDER_WINDOW_TEXT_Y =  2,
   GENDER_WINDOW_TILE_X = (GENDER_WINDOW_TEXT_X / TILE_WIDTH),
   GENDER_WINDOW_TILE_Y = (GENDER_WINDOW_TEXT_Y / TILE_HEIGHT),
   GENDER_WINDOW_TILE_W = 1,
   GENDER_WINDOW_TILE_H = 2,
   GENDER_WINDOW_TILE_COUNT = GENDER_WINDOW_TILE_W * GENDER_WINDOW_TILE_H,
};
vram_bg_layout {
   vram_bg_tilemap tilemaps[4]; // 256 tiles
   
   vram_bg_tile blank_tile;
   vram_bg_tile backdrop_tiles[sizeof(sBGTileGfx) / TILE_SIZE_4BPP];
   vram_bg_tile backdrop_gender_tiles[sizeof(sBGGenderTiles) / TILE_SIZE_4BPP];
   vram_bg_tile menu_button_tiles[sizeof(sMenuButtonFaceGfx) / TILE_SIZE_4BPP];
   vram_bg_tile keyboard_body[VUIKEYBOARD_WINDOW_TILE_COUNT];
   vram_bg_tile user_window_frame[9];
   vram_bg_tile keyboard_value[VUIKEYBOARDVALUE_WINDOW_TILE_COUNT];
   struct {
      vram_bg_tile button_backspace[3*4];
      vram_bg_tile button_ok[3*4];
      vram_bg_tile gender[GENDER_WINDOW_TILE_COUNT];
      vram_bg_tile title[TITLE_WINDOW_TILE_COUNT];
   } windows;
};
__verify_vram_bg_layout;

static const struct BgTemplate sBgTemplates[] = {
   {
      .bg            = BGLAYER_BACKDROP,
      .charBaseIndex = 0,
      .mapBaseIndex  = V_MAP_BASE(tilemaps[BGLAYER_BACKDROP]),
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 3,
      .baseTile      = 0
   },
   {
      .bg            = BGLAYER_BUTTONS,
      .charBaseIndex = 0,
      .mapBaseIndex  = V_MAP_BASE(tilemaps[BGLAYER_BUTTONS]),
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 2,
      .baseTile      = 0
   },
   {
      .bg            = BGLAYER_TEXT,
      .charBaseIndex = 0,
      .mapBaseIndex  = V_MAP_BASE(tilemaps[BGLAYER_TEXT]),
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 1,
      .baseTile      = 0
   },
};

#define MENU_STATE gShortStringEntryMenuState

static const u8 sButtonLabel_OK[]        = _("OK");
static const u8 sButtonLabel_Backspace[] = _("Del.");
static const u8 sKeypadIconTiles[] = INCBIN_U8("graphics/fonts/keypad_icons.4bpp");

static void Task_WaitFadeIn(u8);
static void Task_OnFrame(u8);
static void Task_BeginExit(u8);
static void Task_WaitFadeOut(u8);
static void Teardown(void);

static bool8 IsNicknamingPokemon(void);

static void OnCharsetChanged(void);
static void OnTextEntryChanged(const u8*);
static void OnTextEntryFull(void);
static void OnButtonCharset_Upper(void);
static void OnButtonCharset_Lower(void);
static void OnButtonCharset_Symbol(void);
static void OnButtonCharset_AccentUpper(void);
static void OnButtonCharset_AccentLower(void);
static void OnButtonOK(void);
static void OnButtonBackspace(void);

static void InitCB2(void);
static void MainCB2(void);
static void VBlankCB(void);

static void PaintGenderIcon(void);

static void PaintTitleText(void);

// -----------------------------------------------------------------------

extern void OpenShortStringEntryMenu(const struct ShortStringEntryMenuParams* params) {
   SetVBlankCallback(NULL);
   SetHBlankCallback(NULL);
   LuUI_ResetBackgroundsAndVRAM();
   LuUI_ResetSpritesAndEffects();
   FreeAllWindowBuffers();
   InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
   DeactivateAllTextPrinters();
   
   ShortStringEntryMenu_CreateState();
   ShortStringEntryMenu_InitState(params);
   SetMainCallback2(InitCB2);
   gMain.state = 0;
}

static const VUITextColors sPlainTextColors = { 0, 2, 3 };

static void ClearBGTilemap(u8 bg, u8 palette) {
   FillBgTilemapBufferRect(
      bg,
      V_TILE_ID(blank_tile),
      0, 0, 256/TILE_WIDTH, 256/TILE_HEIGHT,
      palette
   );
}

enum {
   INITSTATE_PREP_BACKGROUND_LAYERS = 0,
   INITSTATE_CLEAR_TEXT_BG_LAYER,
   INITSTATE_DRAW_BACKGROUND_LAYERS,
   INITSTATE_LOAD_PLAYER_WINDOW_FRAME,
   INITSTATE_PREP_MENU_BUTTONS,
   INITSTATE_PREP_CHARSET_BUTTONS,
   INITSTATE_CREATE_TASK,
   INITSTATE_INIT_WIDGETS,
   INITSTATE_TITLE_AND_ICONS,
   INITSTATE_START_RUNNING_MENU,
};
static void InitCB2(void) {
   AGB_ASSERT(MENU_STATE != NULL);
   switch (gMain.state) {
      case INITSTATE_PREP_BACKGROUND_LAYERS:
         ShowBg(BGLAYER_BACKDROP);
         ShowBg(BGLAYER_BUTTONS);
         ShowBg(BGLAYER_TEXT);
         SetBgTilemapBuffer(BGLAYER_BACKDROP, MENU_STATE->tilemap_buffers[BGLAYER_BACKDROP]);
         SetBgTilemapBuffer(BGLAYER_BUTTONS,  MENU_STATE->tilemap_buffers[BGLAYER_BUTTONS]);
         SetBgTilemapBuffer(BGLAYER_TEXT,     MENU_STATE->tilemap_buffers[BGLAYER_TEXT]);
         {
            const struct WindowTemplate dummy[] = { DUMMY_WIN_TEMPLATE };
            InitWindows(dummy);
         }
         
         SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
         
         gMain.state++;
         break;
      case INITSTATE_CLEAR_TEXT_BG_LAYER:
         LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(PALETTE_ID_TEXT), PLTT_SIZE_4BPP);
         ClearBGTilemap(BGLAYER_TEXT, PALETTE_ID_TEXT);
         CopyBgTilemapBufferToVram(BGLAYER_TEXT);
         gMain.state++;
         break;
      case INITSTATE_DRAW_BACKGROUND_LAYERS:
         //ResetTasks(); // we're called from a task-based menu
         
         V_LOAD_TILES(BGLAYER_BACKDROP, backdrop_tiles,    sBGTileGfx);
         PrepBgTilemap(BGLAYER_BACKDROP, (u16*)sBGTilemap, sizeof(sBGTilemap), V_TILE_ID(backdrop_tiles));
         LoadPalette(sBGPalette, BG_PLTT_ID(PALETTE_ID_BACKDROP), sizeof(sBGPalette));
         //
         // If we're going to be drawing a gender symbol, then set up the alternate 
         // backdrop tiles at the spot where we draw it:
         //
         if (MENU_STATE->gender != MON_GENDERLESS) {
            V_LOAD_TILES(BGLAYER_BACKDROP, backdrop_gender_tiles, sBGGenderTiles);
            WriteSequenceToBgTilemapBuffer(
               BGLAYER_BACKDROP,
               V_TILE_ID(backdrop_gender_tiles),
               24 / TILE_WIDTH,
               0  / TILE_HEIGHT,
               32 / TILE_WIDTH,
               32 / TILE_HEIGHT,
               PALETTE_ID_BACKDROP,
               1
            );
         }
         CopyBgTilemapBufferToVram(BGLAYER_BACKDROP);
         
         gMain.state++;
         break;
      case INITSTATE_LOAD_PLAYER_WINDOW_FRAME:
         LuUI_LoadPlayerWindowFrame(
            BGLAYER_TEXT,
            PALETTE_ID_USER_BORDER,
            V_TILE_ID(user_window_frame)
         );
         gMain.state++;
         break;
      case INITSTATE_PREP_MENU_BUTTONS:
         LoadPalette(sMenuButtonFacePal, BG_PLTT_ID(PALETTE_ID_MENUBUTTON), sizeof(sMenuButtonFacePal));
         V_LOAD_TILES(BGLAYER_BUTTONS, menu_button_tiles, sMenuButtonFaceGfx);
         ClearBGTilemap(BGLAYER_BUTTONS, PALETTE_ID_MENUBUTTON);
         
         ChangeBgY(BGLAYER_BUTTONS, -4 << 8, BG_COORD_SET);
         {  // Button face graphics
            const int size =  5; // 5x5-tile buttons
            const int x    = 23;
            const u8  y[2] = { 6, 6 + size };
            //
            for(int i = 0; i < 2; ++i) {
               WriteSequenceToBgTilemapBuffer(
                  BGLAYER_BUTTONS,
                  V_TILE_ID(menu_button_tiles),
                  x,
                  y[i],
                  size,
                  size,
                  PALETTE_ID_MENUBUTTON,
                  1
               );
            }
         }
         {  // Button text
            const VUITextColors text_colors = { 0, 13, 15 };
            const struct Bitmap src_bitmap  = {
               .pixels = sKeypadIconTiles,
               .width  = 128,
               .height =  32,
            };
            const u8 keypad_icon_remapping[16] = {
                0, // transparent
               13, // keypad button face
               15, // keypad button lettering + shadow
                7, // keypad button lettering anti-alias
                5, // keypad button D-Pad direction indicator
               6, 7, 8, 9, 10, 11, 12, 13, 14, 15
            };
            
            {  // Button text: OK
               const struct WindowTemplate tmpl = {
                  .bg          = BGLAYER_TEXT,
                  .tilemapLeft = WIN_BTN_OK_X / TILE_WIDTH,
                  .tilemapTop  = WIN_BTN_OK_Y / TILE_HEIGHT,
                  .width       = 3,
                  .height      = 4,
                  .paletteNum  = PALETTE_ID_MENUBUTTON,
                  .baseBlock   = V_TILE_ID(windows.button_ok)
               };
               u8 window_id = MENU_STATE->window_ids.button_label_ok = AddWindow(&tmpl);
               AGB_ASSERT(window_id != WINDOW_NONE);
               PutWindowTilemap(window_id);
               FillWindowPixelBuffer(window_id, PIXEL_FILL(0));
               
               AddTextPrinterParameterized3(
                  window_id,
                  FONT_NORMAL,
                  6,
                  3,
                  text_colors.list,
                  TEXT_SKIP_DRAW,
                  sButtonLabel_OK
               );
               
               //
               // Draw the "Start" button.
               //
               struct Bitmap dst_bitmap = {
                  .pixels = gWindows[window_id].tileData,
                  .width  = gWindows[window_id].window.width  * TILE_WIDTH,
                  .height = gWindows[window_id].window.height * TILE_HEIGHT,
               };
               BlitBitmapRect4BitRemapped(
                  &src_bitmap,
                  &dst_bitmap,
                  49,  4, // XxY src
                   1, 17, // XxY dst
                  23,  8, // WxH
                  keypad_icon_remapping
               );
               
               CopyWindowToVram(window_id, COPYWIN_FULL);
            }
            {  // Button text: Backspace
               const struct WindowTemplate tmpl = {
                  .bg          = BGLAYER_TEXT,
                  .tilemapLeft = WIN_BTN_BKSP_X / TILE_WIDTH,
                  .tilemapTop  = WIN_BTN_BKSP_Y / TILE_HEIGHT,
                  .width       = 3,
                  .height      = 4,
                  .paletteNum  = PALETTE_ID_MENUBUTTON,
                  .baseBlock   = V_TILE_ID(windows.button_backspace)
               };
               u8 window_id = MENU_STATE->window_ids.button_label_backspace = AddWindow(&tmpl);
               AGB_ASSERT(window_id != WINDOW_NONE);
               PutWindowTilemap(window_id);
               FillWindowPixelBuffer(window_id, PIXEL_FILL(0));
               
               AddTextPrinterParameterized3(
                  window_id,
                  FONT_NORMAL,
                  3,
                  4,
                  text_colors.list,
                  TEXT_SKIP_DRAW,
                  sButtonLabel_Backspace
               );
               
               //
               // Draw the "B" button.
               //
               struct Bitmap dst_bitmap = {
                  .pixels = gWindows[window_id].tileData,
                  .width  = gWindows[window_id].window.width  * TILE_WIDTH,
                  .height = gWindows[window_id].window.height * TILE_HEIGHT,
               };
               BlitBitmapRect4BitRemapped(
                  &src_bitmap,
                  &dst_bitmap,
                  8,  4, // XxY src
                  8, 20, // XxY dst
                  8,  8, // WxH
                  keypad_icon_remapping
               );
               
               CopyWindowToVram(window_id, COPYWIN_FULL);
            }
         }
         CopyBgTilemapBufferToVram(BGLAYER_BUTTONS);
         
         gMain.state++;
         break;
      case INITSTATE_PREP_CHARSET_BUTTONS:
         ShortStringEntryMenu_SetUpCharsetButtons(&MENU_STATE->vui.widgets.charset_buttons);
         ShortStringEntryMenu_UpdateSelectedCharsetButtonSprite(&MENU_STATE->vui.widgets.charset_buttons, SHORTSTRINGENTRY_CHARSET_UPPER);
         gMain.state++;
         break;
      case INITSTATE_CREATE_TASK:
         MENU_STATE->task_id = CreateTask(Task_WaitFadeIn, 0);
         gMain.state++;
         break;
      case INITSTATE_INIT_WIDGETS:
         {  // keyboard value
            const struct VUIKeyboardValue_InitParams params = {
               .bg_layer      = BGLAYER_TEXT,
               .palette       = PALETTE_ID_TEXT,
               .colors        = sPlainTextColors,
               .frame         = NULL,
               .tile_x        = VALUE_TILE_X,
               .tile_y        = VALUE_TILE_Y,
               .first_tile_id = V_TILE_ID(keyboard_value),
               .max_length    = MENU_STATE->max_length
            };
            VUIKeyboardValue_Initialize(&MENU_STATE->vui.widgets.value, &params);
         }
         {  // keyboard
            const struct VUICustomKeyboard_InitParams params = {
               .buffer = {
                  .data = MENU_STATE->buffer,
                  .size = MENU_STATE->max_length,
               },
               .callbacks = {
                  .on_text_changed      = OnTextEntryChanged,
                  .on_text_at_maxlength = OnTextEntryFull,
               },
               .charsets       = gShortStringEntryMenuCharsets,
               .charsets_count = ARRAY_COUNT(gShortStringEntryMenuCharsets),
               .frame          = NULL,
               .bg_layer      = BGLAYER_TEXT,
               .palette       = PALETTE_ID_TEXT,
               .colors        = sPlainTextColors,
               .tile_x        = KEYBOARD_TILE_X,
               .tile_y        = KEYBOARD_TILE_Y,
               .first_tile_id = V_TILE_ID(keyboard_body),
            };
            VUICustomKeyboard_Initialize(&MENU_STATE->vui.widgets.keyboard, &params);
         }
         {  // menu buttons
            {  // OK
               const struct VUIButton_InitParams params = {
                  .callbacks = {
                     .on_press = OnButtonOK,
                  },
               };
               VUIButton_Initialize(&MENU_STATE->vui.widgets.button_ok, &params);
            }
            {  // Backspace
               const struct VUIButton_InitParams params = {
                  .callbacks = {
                     .on_press = OnButtonBackspace,
                  },
               };
               VUIButton_Initialize(&MENU_STATE->vui.widgets.button_backspace, &params);
            }
         }
         {  // charsets
            auto buttons = &MENU_STATE->vui.widgets.charset_buttons;
            buttons->upper.callbacks.on_press = OnButtonCharset_Upper;
            buttons->lower.callbacks.on_press = OnButtonCharset_Lower;
            buttons->symbol.callbacks.on_press = OnButtonCharset_Symbol;
            buttons->accent_u.callbacks.on_press = OnButtonCharset_AccentUpper;
            buttons->accent_l.callbacks.on_press = OnButtonCharset_AccentLower;
         }
         ShortStringEntryMenu_SetUpWidgetGrid(MENU_STATE);
         gMain.state++;
         break;
      case INITSTATE_TITLE_AND_ICONS:
         ShortStringEntryMenu_SetUpCursors(MENU_STATE);
         PaintTitleText();
         PaintGenderIcon();
         MENU_STATE->sprite_ids.icon = ShortStringEntryMenu_ConstructIcon(&MENU_STATE->icon);
         gMain.state++;
         break;
      case INITSTATE_START_RUNNING_MENU:
         BeginNormalPaletteFade(PALETTES_ALL, -1, 16, 0, RGB_BLACK);
         SetVBlankCallback(VBlankCB);
         SetMainCallback2(MainCB2);
         return;
   }
}

// -----------------------------------------------------------------------

static void Task_WaitFadeIn(u8 task_id) {
   if (!gPaletteFade.active) {
      VUIKeyboardValue* widget = &MENU_STATE->vui.widgets.value;
      VUIKeyboardValue_SetUnderscoreVisibility(widget, TRUE);
      VUIKeyboardValue_ShowValue              (widget, MENU_STATE->buffer);
      
      gTasks[task_id].func = Task_OnFrame;
   }
}
static void Task_OnFrame(u8 task_id) {
   VUICustomKeyboard* keyboard = &MENU_STATE->vui.widgets.keyboard;
   if (JOY_NEW(START_BUTTON)) {
      VUIContext_FocusWidget(&MENU_STATE->vui.context, (VUIWidget*)&MENU_STATE->vui.widgets.button_ok);
   } else if (JOY_NEW(SELECT_BUTTON)) {
      VUICustomKeyboard_NextCharset(keyboard);
   } else if (JOY_NEW(B_BUTTON)) {
      VUICustomKeyboard_Backspace(keyboard);
   } else if (JOY_NEW(L_BUTTON)) {
      u8 charset = keyboard->charset;
      if (charset == 0) {
         charset = keyboard->charsets_count - 1;
      } else {
         --charset;
      }
      VUICustomKeyboard_SetCharset(keyboard, charset);
      OnCharsetChanged();
   } else if (JOY_NEW(R_BUTTON)) {
      VUICustomKeyboard_NextCharset(keyboard);
      OnCharsetChanged();
   } else {
      auto prior_focus = MENU_STATE->vui.context.focused;
      VUIContext_HandleInput(&MENU_STATE->vui.context);
      if (!MENU_STATE || MENU_STATE->task_id == TASK_NONE) {
         //
         // User triggered menu exit.
         //
         return;
      }
      if (MENU_STATE->vui.context.focused != prior_focus) {
         ShortStringEntryMenu_UpdateCursors(MENU_STATE);
      }
      //
      // Each harset button has a background and outline color done 
      // up in its own color within the palette. We set all such 
      // colors to white by default, and this means we can show an 
      // outline around the button the cursor is over, or shade in 
      // the currently active button, by just messing with the 
      // palette.
      //
      {
         const u16 SELECTED_BACK = ( 0 << 10) | (20 << 5) | 31; // BGR colors
         
         /*//
         u8 palette_id = IndexOfSpritePaletteTag(SPRITE_PAL_TAG_CHARSET_LABEL);
         if (palette_id != 0xFF) {
            const u16 base_bgcolor = OBJ_PLTT_ID(palette_id) + 8;
            const u16 base_fgcolor = OBJ_PLTT_ID(palette_id) + 2;
            for(u8 i = 0; i < 5; ++i) {
               VUIWidget* button = (VUIWidget*)&MENU_STATE->vui.widgets.charset_buttons.list[i];
               if (keyboard->charset == i) {
                  gPlttBufferFaded[base_bgcolor + i] = SELECTED_BACK;
               } else {
                  gPlttBufferFaded[base_bgcolor + i] = RGB_WHITE;
               }
               if (MENU_STATE->vui.context.focused == (VUIWidget*)button) {
                  gPlttBufferFaded[base_fgcolor + i] = RGB_RED;
               } else {
                  gPlttBufferFaded[base_fgcolor + i] = gPlttBufferFaded[base_bgcolor + i];
               }
            }
         }
         //*/
      }
   }
}
static void Task_BeginExit(u8 task_id) {
   //
   // TODO: If we've been asked to show a dialog box on exit, then 
   // here wait for it to be dismissed.
   //
   BeginNormalPaletteFade(PALETTES_ALL, 1, 0, 16, RGB_BLACK);
   gTasks[MENU_STATE->task_id].func = Task_WaitFadeOut;
}
static void Task_WaitFadeOut(u8 task_id) {
   if (gPaletteFade.active) {
      return;
   }
   
   void(*callback)(const u8*) = MENU_STATE->callback;
   //
   u8 local_value[VUIKEYBOARDVALUE_MAX_SUPPORTED_SIZE + 1];
   memset(local_value, EOS, sizeof(local_value));
   StringCopy(local_value, MENU_STATE->buffer);
   Teardown();
   //
   (callback)(local_value);
}
static void Teardown(void) {
   DebugPrintf("[LuNamingScreen][Teardown] Tearing down...");
   ShortStringEntryMenu_DestroyState();
   DebugPrintf("[LuNamingScreen][Teardown] Freeing window buffers...");
   FreeAllWindowBuffers();
   DebugPrintf("[LuNamingScreen][Teardown] Done.");
}


static bool8 IsNicknamingPokemon(void) {
   return MENU_STATE->icon.type == SHORTSTRINGENTRY_ICONTYPE_POKEMON;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void OnCharsetChanged(void) {
   ShortStringEntryMenu_UpdateSelectedCharsetButtonSprite(
      &MENU_STATE->vui.widgets.charset_buttons,
      MENU_STATE->vui.widgets.keyboard.charset
   );
}
static void OnTextEntryChanged(const u8* value) {
   VUIKeyboardValue_ShowValue(&MENU_STATE->vui.widgets.value, value);
}
static void OnTextEntryFull(void) {
   PlaySE(SE_FAILURE);
}
static void OnButtonCharset_Upper(void) {
   VUICustomKeyboard_SetCharset(&MENU_STATE->vui.widgets.keyboard, 0);
   OnCharsetChanged();
}
static void OnButtonCharset_Lower(void) {
   VUICustomKeyboard_SetCharset(&MENU_STATE->vui.widgets.keyboard, 1);
   OnCharsetChanged();
}
static void OnButtonCharset_Symbol(void) {
   VUICustomKeyboard_SetCharset(&MENU_STATE->vui.widgets.keyboard, 2);
   OnCharsetChanged();
}
static void OnButtonCharset_AccentUpper(void) {
   VUICustomKeyboard_SetCharset(&MENU_STATE->vui.widgets.keyboard, 3);
   OnCharsetChanged();
}
static void OnButtonCharset_AccentLower(void) {
   VUICustomKeyboard_SetCharset(&MENU_STATE->vui.widgets.keyboard, 4);
   OnCharsetChanged();
}
static void OnButtonOK(void) {
   //
   // TODO: If we've been asked to show a dialog box on exit, then 
   // here display it.
   //
   gTasks[MENU_STATE->task_id].func = Task_BeginExit;
}
static void OnButtonBackspace(void) {
   VUICustomKeyboard_Backspace(&MENU_STATE->vui.widgets.keyboard);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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

// -----------------------------------------------------------------------

static const u8 sGenderColors[2][3] = {
   { 0, TEXT_COLOR_LIGHT_BLUE, TEXT_COLOR_BLUE },
   { 0, TEXT_COLOR_LIGHT_RED,  TEXT_COLOR_RED  }
};
static void PaintGenderIcon() {
   if (MENU_STATE->gender == MON_GENDERLESS)
      return;
   if (MENU_STATE->icon.type != SHORTSTRINGENTRY_ICONTYPE_POKEMON)
      //
      // The gender value may be present for OWs; the main use case there is 
      // when starting a new game. When the player picks a gender, we pop a 
      // naming screen, and we need it to show the gender the player just 
      // chose, not whatever gender is leftover in SaveBlock2.
      //
      return;
   
   u8 window_id = MENU_STATE->window_ids.gender;
   if (window_id == WINDOW_NONE) {
      const struct WindowTemplate tmpl = {
         .bg          = BGLAYER_TEXT,
         .tilemapLeft = GENDER_WINDOW_TILE_X,
         .tilemapTop  = GENDER_WINDOW_TILE_Y,
         .width       = GENDER_WINDOW_TILE_W,
         .height      = GENDER_WINDOW_TILE_H,
         .paletteNum  = PALETTE_ID_TEXT,
         .baseBlock   = V_TILE_ID(windows.gender)
      };
      
      window_id = MENU_STATE->window_ids.gender = AddWindow(&tmpl);
      if (window_id == WINDOW_NONE)
         return;
      PutWindowTilemap(window_id);
   }
   
   const u8* text = gText_MaleSymbol;
   u8 color_idx = 0;
   if (MENU_STATE->gender == MON_FEMALE) {
      color_idx = 1;
      text      = gText_FemaleSymbol;
   }
   
   FillWindowPixelBuffer(window_id, PIXEL_FILL(sGenderColors[color_idx][0]));
   AddTextPrinterParameterized3(
      window_id,
      FONT_NORMAL,
      GENDER_WINDOW_TEXT_X % TILE_WIDTH,
      GENDER_WINDOW_TEXT_Y % TILE_HEIGHT,
      sGenderColors[color_idx],
      TEXT_SKIP_DRAW,
      text
   );
   CopyWindowToVram(window_id, COPYWIN_FULL);
}

static const u8 sDefaultTitle[] = _("Enter a name.");static const u8 sNicknameTitle[] = _("{STR_VAR_1}'s nickname?");
//
static void PaintTitleText(void) {
   u8 window_id = MENU_STATE->window_ids.title;
   if (window_id == WINDOW_NONE) {
      const struct WindowTemplate tmpl = {
         .bg          = BGLAYER_TEXT,
         .tilemapLeft = TITLE_WINDOW_TILE_X,
         .tilemapTop  = 0,
         .width       = TITLE_WINDOW_TILE_WIDTH,
         .height      = TITLE_WINDOW_TILE_HEIGHT,
         .paletteNum  = PALETTE_ID_TEXT,
         .baseBlock   = V_TILE_ID(windows.title)
      };
      
      window_id = MENU_STATE->window_ids.title = AddWindow(&tmpl);
      if (window_id == WINDOW_NONE)
         return;
      PutWindowTilemap(window_id);
   }
   
   FillWindowPixelBuffer(window_id, PIXEL_FILL(0));
   
   u8 buffer[32];
   const u8* text = MENU_STATE->title;
   if (text == NULL) {
      text = sDefaultTitle;
      if (IsNicknamingPokemon()) {
         buffer[0] = EOS;
         StringCopy(buffer, gSpeciesNames[MENU_STATE->icon.pokemon.species]);
         StringAppend(buffer, gText_PkmnsNickname);
         text = buffer;
      }
   }
   
   AddTextPrinterParameterized3(
      window_id,
      FONT_BOLD,
      6,
      2,
      sPlainTextColors.list,
      TEXT_SKIP_DRAW,
      text
   );
   
   CopyWindowToVram(window_id, COPYWIN_FULL);
}