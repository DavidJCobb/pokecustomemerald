#include "lu/naming_screen.h"
#include "lu/vui/vui-context.h"
#include "lu/vui/keyboard.h"
#include "lu/vui/keyboard-value.h"
#include "lu/vui/sprite-button.h"
#include "gba/gba.h"
#include "gba/isagbprint.h"
#include "bg.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "palette.h"
#include "sound.h" // PlaySE
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h" // GetTextWindowPalette
#include "window.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "constants/songs.h" // SE_SELECT and other sound effect constants
#include "lu/ui_helpers.h"
#include "lu/vram_layout_helpers_new.h"

static const u8 sBlankBGTile[] = INCBIN_U8("graphics/lu/cgo_menu/bg-tile-blank.4bpp"); // color 1
enum {
   PALETTE_ID_TEXT = 15,
};
vram_bg_layout {
   __vram_bg_tilemap tilemaps[4];
   
   __vram_bg_tile blank_tile;
   __vram_bg_tile keyboard_borders[8];
   __vram_bg_tile keyboard_body[VUIKEYBOARD_WINDOW_TILE_COUNT];
   __vram_bg_tile user_window_frame[9];
   __vram_bg_tile keyboard_value[VUIKEYBOARDVALUE_WINDOW_TILE_COUNT];
};
__verify_vram_bg_layout;

static const struct BgTemplate sBgTemplates[] = {
   {
      .bg            = 0,
      .charBaseIndex = 0,
      .mapBaseIndex  = 1,
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 1,
      .baseTile      = 0
   },
};

struct MenuState {
   u8 task_id;
   void(*callback)(const u8*);
   u8 buffer[VUIKEYBOARDVALUE_MAX_SUPPORTED_SIZE + 1];
   struct {
      VUIContext context;
      struct {
         VUIKeyboard      keyboard;
         VUIKeyboardValue value;
         VUISpriteButton  button_ok;
         VUISpriteButton  button_backspace;
         VUISpriteButton  button_charset;
      } widgets;
      VUIWidget* widget_list[5];
   } vui;
};
static EWRAM_DATA struct MenuState* sMenuState = NULL;

static void InitState(const struct LuNamingScreenParams*);
static void Task_WaitFadeIn(u8);
static void Task_OnFrame(u8);
static void Teardown(void);

static void OnTextEntryChanged(const u8*);
static void OnTextEntryFull(void);
static void OnButtonCharset(void);
static void OnButtonOK(void);

static void InitCB2(void);
static void MainCB2(void);
static void VBlankCB(void);

// -----------------------------------------------------------------------

extern void LuNamingScreen(const struct LuNamingScreenParams* params) {
   SetVBlankCallback(NULL);
   SetHBlankCallback(NULL);
   LuUI_ResetBackgroundsAndVRAM();
   LuUI_ResetSpritesAndEffects();
   FreeAllWindowBuffers();
   InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
   DeactivateAllTextPrinters();
   
   InitState(params);
   SetMainCallback2(InitCB2);
}

// -----------------------------------------------------------------------

static const VUITextColors text_colors = { 1, 2, 3 };

static void InitState(const struct LuNamingScreenParams* params) {
   AGB_ASSERT(!sMenuState);
   sMenuState = AllocZeroed(sizeof(struct MenuState));
   
   u8 max_length = params->max_length;
   AGB_WARNING(max_length <= sizeof(sMenuState->buffer) - 1);
   if (max_length >= sizeof(sMenuState->buffer)) {
      max_length = sizeof(sMenuState->buffer) - 1;
   }
   
   sMenuState->callback = params->callback;
   memset(sMenuState->buffer, EOS, sizeof(sMenuState->buffer));
   if (params->initial_value) {
      u8* end  = StringCopy(sMenuState->buffer, params->initial_value);
      u16 size = end - sMenuState->buffer;
      AGB_WARNING(size <= sizeof(sMenuState->buffer));
   }
   
   sMenuState->vui.widget_list[0] = (VUIWidget*)&sMenuState->vui.widgets.keyboard;
   sMenuState->vui.widget_list[1] = (VUIWidget*)&sMenuState->vui.widgets.value;
   sMenuState->vui.widget_list[2] = (VUIWidget*)&sMenuState->vui.widgets.button_ok;
   sMenuState->vui.widget_list[3] = (VUIWidget*)&sMenuState->vui.widgets.button_backspace;
   sMenuState->vui.widget_list[4] = (VUIWidget*)&sMenuState->vui.widgets.button_charset;
   //
   {
      VUIContext* context = &sMenuState->vui.context;
      context->widgets.list = sMenuState->vui.widget_list;
      context->widgets.size = 4;
      context->w = 2;
      context->h = 3;
      context->allow_wraparound_x = context->allow_wraparound_y = TRUE;
   }
   {
      VUIKeyboardValue* widget = &sMenuState->vui.widgets.value;
      
      const struct VUIKeyboardValue_InitParams params = {
         .bg_layer = 0,
         .palette  = PALETTE_ID_TEXT,
         .colors   = text_colors,
         .tile_x = 0,
         .tile_y = 0,
         .first_tile_id = V_TILE_ID(keyboard_value),
         .max_length    = max_length
      };
      VUIKeyboardValue_Construct(widget, &params);
   }
   {
      VUIKeyboard* widget = &sMenuState->vui.widgets.keyboard;
      
      const struct VUIKeyboard_InitParams params = {
         .bg_layer = 0,
         .palette  = PALETTE_ID_TEXT,
         .colors   = text_colors,
         .tile_x = 0,
         .tile_y = 5,
         .first_tile_id = V_TILE_ID(keyboard_body),
      };
      VUIKeyboard_Construct(widget, &params);
      VUIWidget_SetGridMetrics(widget, 0, 0, 1, 3);
      
      widget->value.buffer     = sMenuState->buffer;
      widget->value.max_length = max_length;
      widget->callbacks.on_text_changed      = OnTextEntryChanged;
      widget->callbacks.on_text_at_maxlength = OnTextEntryFull;
   }
   {
      VUISpriteButton* widget = &sMenuState->vui.widgets.button_ok;
      VUISpriteButton_Construct(widget);
      VUIWidget_SetGridMetrics(widget, 1, 0, 1, 1);
      //
      widget->on_press = OnButtonOK;
   }
   {
      VUISpriteButton* widget = &sMenuState->vui.widgets.button_backspace;
      VUISpriteButton_Construct(widget);
      VUIWidget_SetGridMetrics(widget, 1, 1, 1, 1);
   }
   {
      VUISpriteButton* widget = &sMenuState->vui.widgets.button_charset;
      VUISpriteButton_Construct(widget);
      VUIWidget_SetGridMetrics(widget, 1, 2, 1, 1);
   }
}
static void Task_WaitFadeIn(u8 task_id) {
   if (!gPaletteFade.active) {
      VUIKeyboardValue_SetUnderscoreVisibility(&sMenuState->vui.widgets.value, TRUE);
      VUIKeyboardValue_ShowValue(&sMenuState->vui.widgets.value, sMenuState->buffer);
      gTasks[task_id].func = Task_OnFrame;
   }
}
static void Task_OnFrame(u8 task_id) {
   if (JOY_NEW(START_BUTTON)) {
      VUIContext_FocusWidget(&sMenuState->vui.context, (VUIWidget*)&sMenuState->vui.widgets.button_ok);
   } else if (JOY_NEW(SELECT_BUTTON)) {
      VUIKeyboard_NextCharset(&sMenuState->vui.widgets.keyboard);
   } else if (JOY_NEW(B_BUTTON)) {
      VUIKeyboard_Backspace(&sMenuState->vui.widgets.keyboard);
   } else {
      VUIContext_HandleInput(&sMenuState->vui.context);
   }
}
static void Teardown(void) {
   DebugPrintf("[LuNamingScreen][Teardown] Tearing down...");
   if (sMenuState) {
      DebugPrintf("[LuNamingScreen][Teardown] Beginning to destroy menu state at %08X...", sMenuState);
      DebugPrintf("[LuNamingScreen][Teardown] Destroying task...");
      DestroyTask(sMenuState->task_id);
      DebugPrintf("[LuNamingScreen][Teardown] Destroying all widgets...");
      vui_context_foreach(&sMenuState->vui.context, widget) {
         DebugPrintf("[LuNamingScreen][Teardown] Destroying widget %08X...", widget);
         if (widget)
            VUIWidget_Destroy(widget);
      }
      DebugPrintf("[LuNamingScreen][Teardown] Freeing menu state...");
      Free(sMenuState);
      sMenuState = NULL;
   }
   DebugPrintf("[LuNamingScreen][Teardown] Freeing window buffers...");
   FreeAllWindowBuffers();
   DebugPrintf("[LuNamingScreen][Teardown] Done.");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void OnTextEntryChanged(const u8* value) {
   VUIKeyboardValue_ShowValue(&sMenuState->vui.widgets.value, value);
}
static void OnTextEntryFull(void) {
   PlaySE(SE_FAILURE);
}
static void OnButtonCharset(void) {
   VUIKeyboard_NextCharset(&sMenuState->vui.widgets.keyboard);
}
static void OnButtonOK(void) {
   void(*callback)(const u8*) = sMenuState->callback;
   
   u8 local_value[VUIKEYBOARDVALUE_MAX_SUPPORTED_SIZE + 1];
   memset(local_value, EOS, sizeof(local_value));
   StringCopy(local_value, sMenuState->buffer);
   Teardown();
   
   (callback)(local_value);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void InitCB2(void) {
   switch (gMain.state) {
      default:
      case 0:
         gMain.state++;
         break;
      case 1:
         ShowBg(0);
         
         SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
         
         gMain.state++;
         break;
       case 2:
         //ResetTasks(); // we're called from a task-based menu
         
         V_LOAD_TILES(0, blank_tile, sBlankBGTile);
         
         gMain.state++;
         break;
       case 3:
         LuUI_LoadPlayerWindowFrame(
            0, // BG layer
            1, // palette
            V_TILE_ID(user_window_frame)
         );
         gMain.state++;
         break;
       case 4:
         //LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(BACKGROUND_PALETTE_ID_MENU), sizeof(sOptionMenuBg_Pal));
         gMain.state++;
         break;
       case 5:
         //LoadPalette(sOptionsListingPalette, BG_PLTT_ID(BACKGROUND_PALETTE_ID_TEXT), sizeof(sOptionsListingPalette));
         LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(PALETTE_ID_TEXT), PLTT_SIZE_4BPP);
         gMain.state++;
         break;
       case 6:
         sMenuState->task_id = CreateTask(Task_WaitFadeIn, 0);
         gMain.state++;
         break;
       case 7:
         BeginNormalPaletteFade(PALETTES_ALL, -1, 16, 0, RGB_BLACK);
         SetVBlankCallback(VBlankCB);
         SetMainCallback2(MainCB2);
         return;
   }
}
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