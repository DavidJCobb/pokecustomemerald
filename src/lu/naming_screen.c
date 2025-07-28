#include "lu/naming_screen.h"
#include "lu/vui/vui-context.h"
#include "lu/vui/custom-keyboard.h"
#include "lu/vui/keyboard.h"
#include "lu/vui/keyboard-value.h"
#include "lu/vui/sprite-button.h"
#include "lu/vui/tile-button.h"
#include "gba/gba.h"
#include "gba/isagbprint.h"
#include "bg.h"
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
#include "lu/gfxutils.h"
#include "lu/ui_helpers.h"
#include "lu/vram_layout_helpers_new.h"
//
extern const u8 gSpeciesNames[][POKEMON_NAME_LENGTH + 1]; // from `data.h`

/*//

   TODO:
      
    - VUI: Make it possible to override the directional navigation target of a 
      given widget in a given direction. (Make it a virtual function, perhaps.) 
      Then, use this so that: the keyboard can't navigate down to Upper; but 
      trying to navigate up from Upper still navigates to the keyboard.
      
    - Implement the various cases for which the vanilla naming screen is used.
    
       - Nickname a Pokemon (freshly caught)
       - Nickname a Pokemon (already owned)
       - Change the name of the player-character
       - Change the name of a PC Box
       - Say a password to Walda
   
    - The button mappings shown on "OK" and "Backspace" render as black-on-red 
      because `DrawKeypadIcon` blits directly from the keypad icon tile graphic 
      onto the destination window, using the former's original palette. In order 
      to make the colors of the keypad icons controllable, we'd have to write a 
      blit function that remaps palette indices during the blit.
   
    - Investigate changing Game Freak's BG library, so that it maintains tilemap 
      buffers for all BG layers at all times.
      
       - ONCE WE MAKE THIS CHANGE, WE SHOULD DOCUMENT IT IN THE REPO -- THE CHANGE, 
         ITS MOTIVATIONS AND MECHANICS, AND ITS EFFECTS.
      
       - One of the challenges here is that a BG layer's size can vary; we almost 
         always use 256x256-tile layers, but the following sizes are possible:
         
             128 x  128 (affine only)      256 bytes
             256 x  256                      2 KB
             512 x  256                      4 KB
             256 x  512                      4 KB
             512 x  512                      8 KB
            1024 x 1024 (affine only)       16 KB
            
         Only BG2 and BG3 can be affine, so at most two layers can be 1024px^2.
         
         This means that the maximum possible size of all four backgrounds' tilemaps 
         combined, assuming the largest possible layer sizes, is 48 KB. The typical 
         size for four 256x256px layers will be 8 KB. (For some context, there are 
         64 KB of BG VRAM total.)
         
       - The vanilla codebase uses `gWindowBgTilemapBuffers` to hold tilemap 
         buffers for background layers, whether or not the layers themselves are 
         visible. One buffer is maintained per layer, and its size is equal to 
         `GetBgAttribute(bgLayer, BG_ATTR_METRIC)`. Allocation and management of 
         these buffers is handled by the window library. It attempts to feed the 
         buffers into the BG library (though of course, if the BG layer isn't 
         already visible, then the BG library just ignores that, and the window 
         library misses its chance -- great design...).
         
         I think we should move this code out of the window library and into the 
         background library. When backgrounds are initialized from templates, we 
         should automatically spawn their tilemap buffers; and we should also have 
         an extern function that can be called to force (re)allocation of a layer's 
         tilemap buffer. We should not care one bit whether the background layer 
         is currently visible.
         
         So essentially, we move `gWindowBgTilemapBuffers` into the BG library 
         and rename it, and then get rid of the `sGpuBgConfigs2[n].tilemap` field 
         in favor of the former.
         
          - As documented in more detail below, there are a handful of places 
            where the tilemap buffers are swapped out and wholly owned by more 
            complex systems. The contest and battle UIs come to mind: battles 
            allocate their own tilemap buffers, free them as they see fit, and 
            in general aren't something we want to be tangled up in right now.
            
            We should retain the ability to set the tilemap pointer externally, 
            and have private state bits to keep track of ownership. That is: 
            we should keep the externally accessible `SetBgTilemapBuffer` 
            function, and keep track of whether it's been used. If it has been 
            used, then we assume that the tilemap buffer for that given layer 
            is owned externally, and we never free it ourselves. (When the 
            function is called: if we already have our own buffer for that 
            layer, we should free it.)
            
            This will also allow us to change things incrementally: the API we 
            present externally will be identical to vanilla, except that if 
            you wholly avoid using it, then we do all the work for you.
         
          - A handful of vanilla menus and cutscenes manage their own tilemap 
            buffers, but as far as I can tell, they only even do this because 
            they need BG layers that don't have windows on them. The menus in 
            question don't seem to ever touch the tilemaps directly; they're 
            well-behaved and go through the BG functions for them. So really, 
            we don't need to make any substantial changes to how these menus 
            set up their graphics; if we modify the BG layer to allocate and 
            free its own tilemap buffers as appropriate, then we can just 
            wholly remove the tilemap buffers and code to manage them from 
            these particular menus, and that's that.
            
            Known cases that are, as described, easy to address:
            
             - Battle Factory
             - Battle Records
             - Berry Blender
             - Berry Crush mini-game
             - Contest paintings
             - Dodrio Berry Picking mini-game
             - Cable Car cutscene
             - Credits
             - Egg Hatch cutscene
             - frontier_pass.c
             - Hall of Fame
             - link.c (sLinkErrorBgTilemapBuffer)
             - Pokedex
             - Pokemon Jump mini-game
             - PokeNav: Condition search results
             - PokeNav: List
             - PokeNav: Main menu
             - PokeNav: Match Call GFX
             - PokeNav: Menu Handler GFX
             - PokeNav: Region map
             - PokeNav: Ribbons list
             - PokeNav: Ribbons summary
             - Mail UI
             - Mirage Tower
             - Mystery Gift UI
             - Naming Screen
             - Overworld
             - Union Room Chat UI
             - Wireless communication status screen
            
            Cases that require a bit more work:
            
             - Bag (item_menu.c)
            
             - Battle Dome
            
             - Battle Pyramid bag
            
             - Berry tag screen
            
             - Diploma (they decompress assets directly into it)
            
             - Easy Chat
            
             - Party Menu
            
             - PokeBlock Feed cutscene (pokeblock_feed.c)
            
             - PokeBlock UI (pokeblock.c)
            
             - Pokemon Summary Screen
            
             - PokeNav: Condition screen
             
             - Rayquaza cutscenes (they decompress assets directly into their 
               buffers)
            
             - Roulette
            
             - save_failed_screen.c
             
                - They use gDecompressionBuffer directly, so among other 
                  things, they never even have to free the tilemap buffer.
            
             - Shop menu (shop.c)
               
             - Trade menu (trade.c)
               
             - Trainer Card
               
             - use_pokeblock.c
            
            Even the cases that require "more work" should still be pretty 
            trivial: the BG library has getters for whatever tilemap is 
            loaded, so we can just have these cases call those getters; the 
            tilemap buffers are still reachable from the outside world.
            
            Complex cases:
            
             - Battle UI
             
                - battle_bg.c sets gBattleAnimBgTilemapBuffer as the tilemap 
                  buffer for BG layers 1 and 2.
            
             - Contests
             
                - The main file AND contest_util.c.
            
             - Pokemon Storage System
             
                - They pre-allocate tilemap buffers with specific sizes. Do these 
                  match the parameters they use when setting up the BG layers? If 
                  so, then this oughta be simple. If not, we'll need to look into 
                  that.

//*/

static const u8 sCharsetCharactersUpper[] = __(
   "ABCDEF ."
   "GHIJKL ,"
   "MNOPRQS "
   "TUVWXYZ "
);
static const u8 sCharsetCharactersLower[] = __(
   "abcdef ."
   "ghijkl ,"
   "mnopqrs "
   "tuvwxyz "
);
static const u8 sCharsetCharactersSymbol[] = __(
   "01234 "
   "56789 "
   "!?♂♀/-"
   "…“”‘' "
);
static const u8 sCharsetCharactersAccentUpper[] = __(
   "ÁÂÀÄ  ÇŒ"
   "ÉÊÈË  ßÑ"
   "ÍÎÌÏ    "
   "ÓÔÒÖÚÛÙÜ"
);
static const u8 sCharsetCharactersAccentLower[] = __(
   "áâàä  çœ"
   "éêèë   ñ"
   "íîìï    "
   "óôòöúûùü"
);

static const struct VUICustomKeyboardCharset sCharsets[] = {
   {  // Upper
      .characters = sCharsetCharactersUpper,
      .rows = 4,
      .cols = 8,
      .col_gaps = {
         .count     = 2,
         .positions = { 2, 6 }
      }
   },
   {  // Lower
      .characters = sCharsetCharactersLower,
      .rows = 4,
      .cols = 8,
      .col_gaps = {
         .count     = 2,
         .positions = { 2, 6 }
      }
   },
   {  // Symbol
      .characters = sCharsetCharactersSymbol,
      .rows = 4,
      .cols = 6,
      .col_gaps = {
         .count = 0
      }
   },
   {  // Accent Upper
      .characters = sCharsetCharactersAccentUpper,
      .rows = 4,
      .cols = 8,
      .col_gaps = {
         .count     = 1,
         .positions = { 3 }
      }
   },
   {  // Accent Lower
      .characters = sCharsetCharactersAccentLower,
      .rows = 4,
      .cols = 8,
      .col_gaps = {
         .count     = 1,
         .positions = { 3 }
      }
   },
};

static const struct SpriteSheet   sSpriteSheets[];
static const struct SpritePalette sSpritePalettes[];
static const u8  sBlankBGTile[]     = INCBIN_U8("graphics/lu/cgo_menu/bg-tile-blank.4bpp"); // color 1
static const u8  sBGTiles[]         = INCBIN_U8("graphics/lu/naming_screen/bg.4bpp");
static const u16 sBGPalette[]       = INCBIN_U16("graphics/lu/naming_screen/bg.gbapal");
static const u16 sBackdropPalette[] = INCBIN_U16("graphics/lu/naming_screen/backdrop-jewel-tiles.gbapal");
static const u8  sButtonTiles[]     = INCBIN_U8("graphics/lu/naming_screen/tile-button.4bpp");
static const u16 sButtonPalette[]   = INCBIN_U16("graphics/lu/naming_screen/tile-button.gbapal");
static const struct BGTilemapInfo sBackdropTilemapInfo;
enum {
   MAX_STRING_LENGTH = VUIKEYBOARDVALUE_MAX_SUPPORTED_SIZE,
   
   BGLAYER_BACKDROP = 0,
   BGLAYER_CONTENT  = 1,
   
   PALETTE_ID_CHROME   =  0,
   PALETTE_ID_BUTTON   =  1,
   PALETTE_ID_BACKDROP = 14,
   PALETTE_ID_TEXT     = 15,
   
   SPRITE_GFX_TAG_CHARSET_LABEL = 0x9000,
   SPRITE_PAL_TAG_CHARSET_LABEL = 0x9000,
   SPRITE_PAL_TAG_CUSTOM_ICON   = 0x9001, // preset icons only
   
   BIGBUTTON_TILE_X = 23,
   BIGBUTTON_TILE_Y =  7,
   BIGBUTTON_TILE_W =  5,
   BIGBUTTON_TILE_H =  4,
   BIGBUTTON_WIN_TILE_COUNT = BIGBUTTON_TILE_W * BIGBUTTON_TILE_H,
   
   VALUE_TILE_X = 8,
   VALUE_TILE_Y = 3,
   
   KEYBOARD_TILE_X       = 7,
   KEYBOARD_TILE_Y       = 7,
   KEYBOARD_TILE_INNER_W = VUIKEYBOARD_INNER_W_TILES,
   KEYBOARD_TILE_INNER_H = VUIKEYBOARD_INNER_H_TILES,
   
   TITLE_WINDOW_TILE_X      = 6,
   TITLE_WINDOW_TILE_WIDTH  = DISPLAY_TILE_WIDTH - TITLE_WINDOW_TILE_X - 1,
   TITLE_WINDOW_TILE_HEIGHT = 2,
   TITLE_WINDOW_TILE_COUNT  = TITLE_WINDOW_TILE_WIDTH * TITLE_WINDOW_TILE_HEIGHT,
   
   ICON_BASE_CX = 20, // centerpoint
   ICON_BASE_CY = 16,
   ICON_OFFSET_PKMN_X =  0,
   ICON_OFFSET_PKMN_Y = -4,
   ICON_OFFSET_OW_X   =  0,
   ICON_OFFSET_OW_Y   = -5,
   
   GENDER_WINDOW_TEXT_X = 32,
   GENDER_WINDOW_TEXT_Y =  2,
   GENDER_WINDOW_TILE_X = (GENDER_WINDOW_TEXT_X / TILE_WIDTH),
   GENDER_WINDOW_TILE_Y = (GENDER_WINDOW_TEXT_Y / TILE_HEIGHT),
   GENDER_WINDOW_TILE_W = 1,
   GENDER_WINDOW_TILE_H = 2,
   GENDER_WINDOW_TILE_COUNT = GENDER_WINDOW_TILE_W * GENDER_WINDOW_TILE_H,
};
vram_bg_layout {
   vram_bg_tilemap tilemaps[4];
   
   vram_bg_tile blank_tile;
   vram_bg_tile common_tiles[sizeof(sBGTiles)     / TILE_SIZE_4BPP];
   vram_bg_tile button_tiles[sizeof(sButtonTiles) / TILE_SIZE_4BPP];
   vram_bg_tile keyboard_borders[8];
   vram_bg_tile keyboard_body[VUIKEYBOARD_WINDOW_TILE_COUNT];
   vram_bg_tile user_window_frame[9];
   vram_bg_tile keyboard_value[VUIKEYBOARDVALUE_WINDOW_TILE_COUNT];
   vram_bg_tile backdrop[12];
   struct {
      vram_bg_tile button_backspace[BIGBUTTON_WIN_TILE_COUNT];
      vram_bg_tile button_ok[BIGBUTTON_WIN_TILE_COUNT];
      vram_bg_tile gender[GENDER_WINDOW_TILE_COUNT];
      vram_bg_tile title[TITLE_WINDOW_TILE_COUNT];
   } windows;
};
__verify_vram_bg_layout;

enum {
   COMMONTILE_CHARSETBAR_TOP = V_TILE_ID(common_tiles) + 0,
   COMMONTILE_CHROME_EDGE_L,
   COMMONTILE_TITLEBAR_BACK,
   COMMONTILE_TITLEBAR_DIAGONAL_4,
   
   COMMONTILE_CHARSETBAR_MID,
   COMMONTILE_CHROME_EDGE_TOP,
   COMMONTILE_TITLEBAR_DIAGONAL_2,
   COMMONTILE_TITLEBAR_DIAGONAL_3,
   
   COMMONTILE_CHARSETBAR_BOT,
   COMMONTILE_TITLEBAR_EDGE_BOTTOM_A,     COMMONTILE_CHROME_EDGE_BOT = COMMONTILE_TITLEBAR_EDGE_BOTTOM_A,
   COMMONTILE_TITLEBAR_DIAGONAL_1,
   COMMONTILE_TITLEBAR_EDGE_BOTTOM_B,
   
   COMMONTILE_TRANSPARENT,
   COMMONTILE_CHROME_CORNER_UPPER_A,
   COMMONTILE_CHROME_CORNER_LOWER_A,
   COMMONTILE_UNUSED_15,
   
   COMMONTILE_UNUSED_16,
   COMMONTILE_CHROME_CORNER_UPPER_B,
   COMMONTILE_CHROME_CORNER_LOWER_B,
   COMMONTILE_UNUSED_19,
};

// The abstract-grid layout for our VUI context.
enum {
   /*
      +-------+-------------------------+--------+
      |                                 |   OK   |
      |                Keyboard         +--------+
      |                                 |  Del.  |
      +-------+-------+--------+--------+--------+
      | Upper | Lower | Symbol | ACCENT | accent |
      +-------+-------+--------+--------+--------+
   
   */
   CTXGRID_KEYBOARD_X = 0,
   CTXGRID_KEYBOARD_Y = 0,
   CTXGRID_KEYBOARD_W = 4,
   CTXGRID_KEYBOARD_H = 3,
   
   CTXGRID_BUTTON_OK_X = 4,
   CTXGRID_BUTTON_OK_Y = CTXGRID_KEYBOARD_Y,
   CTXGRID_BUTTON_OK_W = 1,
   CTXGRID_BUTTON_OK_H = CTXGRID_KEYBOARD_H / 2,
   
   CTXGRID_BUTTON_BACK_X = CTXGRID_BUTTON_OK_X,
   CTXGRID_BUTTON_BACK_Y = CTXGRID_BUTTON_OK_Y + CTXGRID_BUTTON_OK_H,
   CTXGRID_BUTTON_BACK_W = CTXGRID_BUTTON_OK_W,
   CTXGRID_BUTTON_BACK_H = CTXGRID_KEYBOARD_H - CTXGRID_BUTTON_BACK_Y,
   
   CTXGRID_CHARSETBUTTON_UPPER_X       = 0,
   CTXGRID_CHARSETBUTTON_LOWER_X       = 1,
   CTXGRID_CHARSETBUTTON_SYMBOL_X      = 2,
   CTXGRID_CHARSETBUTTON_ACCENTUPPER_X = 3,
   CTXGRID_CHARSETBUTTON_ACCENTLOWER_X = 4,
   CTXGRID_CHARSETBUTTON_Y = CTXGRID_KEYBOARD_Y + CTXGRID_KEYBOARD_H,
   
   CTXGRID_W = CTXGRID_BUTTON_OK_X + CTXGRID_BUTTON_OK_W,
   CTXGRID_H = CTXGRID_CHARSETBUTTON_Y + 1,
};

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
      .bg            = BGLAYER_CONTENT,
      .charBaseIndex = 0,
      .mapBaseIndex  = V_MAP_BASE(tilemaps[BGLAYER_CONTENT]),
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
   
   u8 gender;
   struct LuNamingScreenIcon icon;
   const u8* title; // optional
   struct {
      VUIContext context;
      struct {
         VUICustomKeyboard keyboard;
         VUIKeyboardValue  value;
         VUITileButton     button_ok;
         VUITileButton     button_backspace;
         union {
            VUISpriteButton list[5];
            struct {
               VUISpriteButton upper;
               VUISpriteButton lower;
               VUISpriteButton symbol;
               VUISpriteButton accent_u;
               VUISpriteButton accent_l;
            };
         } charset_buttons;
      } widgets;
      VUIWidget* widget_list[9];
   } vui;
   
   union {
      u8 all[6];
      struct {
         u8 charset_label_separators[4];
         u8 charset_labels_button_l;
         u8 charset_labels_button_r;
      };
   } sprite_ids;
   union {
      u8 all[2];
      struct {
         u8 gender;
         u8 title;
      };
   } window_ids;
   
   u8 tilemap_buffers[4][BG_SCREEN_SIZE];
   u8 timer;
};
static EWRAM_DATA struct MenuState* sMenuState = NULL;

static void InitState(const struct LuNamingScreenParams*);
static void Task_WaitFadeIn(u8);
static void Task_OnFrame(u8);
static void Task_BeginExit(u8);
static void Task_WaitFadeOut(u8);
static void Teardown(void);

static bool8 IsNicknamingPokemon(void);

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

static void SetUpGenderIcon(void);

static void PaintTitleBarTiles(void);
static void PaintTitleText(void);
static void SetUpCharsetLabels(void);

static void SetUpIcon(void);

static void DrawBackdrop(void);
static void AnimateBackdrop(void);

static void DrawChromeBorderAround(u8 tile_x, u8 tile_y, u8 tile_w, u8 tile_h);

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

static const u8 sButtonLabel_OK[]          = _("OK");
static const u8 sButtonLabel_Backspace[]   = _("Del.");
static const u8 sButtonMapping_OK[]        = _("{START_BUTTON}");
static const u8 sButtonMapping_Backspace[] = _("{B_BUTTON}");
static void InitState(const struct LuNamingScreenParams* params) {
   AGB_ASSERT(!sMenuState);
   sMenuState = AllocZeroed(sizeof(struct MenuState));
   
   for(u8 i = 0; i < ARRAY_COUNT(sMenuState->sprite_ids.all); ++i)
      sMenuState->sprite_ids.all[i] = SPRITE_NONE;
   for(u8 i = 0; i < ARRAY_COUNT(sMenuState->window_ids.all); ++i)
      sMenuState->window_ids.all[i] = WINDOW_NONE;
   
   sMenuState->gender = MON_GENDERLESS;
   if (params->has_gender)
      sMenuState->gender = params->gender;
   sMenuState->icon  = params->icon;
   sMenuState->title = params->title;
   
   // Why the actual heck do we even need to do this? The BG library should 
   // maintain its own buffers! Every single function meant to interact 
   // with the content of a BG layer assumes that it has a CPU-side tilemap 
   // buffer to write to, and seemingly 99% of the time, Game Freak only 
   // ever sets these buffers up as a one-off side effect of the window 
   // library... and even *that* isn't reliable!
   //
   // (The BG library doesn't maintain its own buffers. The window library 
   // will create buffers for it, but the way it does this is very, very 
   // janky and unreliable. That's without even getting into the fact that 
   // you may not need, or want, a window on a given background layer, yet 
   // may still want to use the layer.)
   //
   // Oh, and for bonus jank: these calls don't even do anything, because 
   // for some reason, this function is conditioned not to do anything 
   // unless the BG layer has been set to visible. Scroll down to the 
   // InitCB2 function to see the real work get done.
   //
   // We really, really, really, really, really should fix up GF's BG code 
   // at some point. This sucks. I hate this.
   ShowBg(BGLAYER_CONTENT);
   SetBgTilemapBuffer(0, sMenuState->tilemap_buffers[0]);
   SetBgTilemapBuffer(1, sMenuState->tilemap_buffers[1]);
   SetBgTilemapBuffer(2, sMenuState->tilemap_buffers[2]);
   SetBgTilemapBuffer(3, sMenuState->tilemap_buffers[3]);
   FillBgTilemapBufferRect(
      BGLAYER_CONTENT,
      V_TILE_ID(common_tiles[12]), // blank
      0, 0,
      DISPLAY_TILE_WIDTH, DISPLAY_TILE_HEIGHT,
      PALETTE_ID_CHROME
   );
   
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
   
   {
      auto list    = sMenuState->vui.widget_list;
      auto widgets = &sMenuState->vui.widgets;
      list[0] = (VUIWidget*)&widgets->keyboard;
      list[1] = (VUIWidget*)&widgets->value;
      list[2] = (VUIWidget*)&widgets->button_ok;
      list[3] = (VUIWidget*)&widgets->button_backspace;
      for(u8 i = 0; i < 5; ++i) {
         list[4 + i] = (VUIWidget*)&widgets->charset_buttons.list[i];
      }
   }
   
   LoadSpriteSheets(sSpriteSheets);
   LoadSpritePalettes(sSpritePalettes);
   
   {  // context
      VUIContext* context = &sMenuState->vui.context;
      context->widgets.list = sMenuState->vui.widget_list;
      context->widgets.size = ARRAY_COUNT(sMenuState->vui.widget_list);
      context->w = CTXGRID_W;
      context->h = CTXGRID_H;
      context->allow_wraparound_x = context->allow_wraparound_y = TRUE;
   }
   {  // keyboard value
      VUIKeyboardValue* widget = &sMenuState->vui.widgets.value;
      
      const struct VUIKeyboardValue_InitParams params = {
         .bg_layer = BGLAYER_CONTENT,
         .palette  = PALETTE_ID_TEXT,
         .colors   = text_colors,
         .tile_x = VALUE_TILE_X,
         .tile_y = VALUE_TILE_Y,
         .first_tile_id = V_TILE_ID(keyboard_value),
         .max_length    = max_length
      };
      VUIKeyboardValue_Construct(widget, &params);
   }
   {  // keyboard
      VUICustomKeyboard* widget = &sMenuState->vui.widgets.keyboard;
      const struct VUICustomKeyboard_InitParams params = {
         .buffer = {
            .data = sMenuState->buffer,
            .size = max_length,
         },
         .callbacks = {
            .on_text_changed      = OnTextEntryChanged,
            .on_text_at_maxlength = OnTextEntryFull,
         },
         .charsets       = sCharsets,
         .charsets_count = ARRAY_COUNT(sCharsets),
         .grid = {
            .pos  = { CTXGRID_KEYBOARD_X, CTXGRID_KEYBOARD_Y },
            .size = { CTXGRID_KEYBOARD_W, CTXGRID_KEYBOARD_H },
         },
         .bg_layer      = BGLAYER_CONTENT,
         .palette       = PALETTE_ID_TEXT,
         .colors        = text_colors,
         .tile_x        = KEYBOARD_TILE_X,
         .tile_y        = KEYBOARD_TILE_Y,
         .first_tile_id = V_TILE_ID(keyboard_body),
      };
      VUICustomKeyboard_Construct(widget, &params);
   }
   {
      const struct VUITileButton_GraphicsParams gfx = {
         .bg      = BGLAYER_CONTENT,
         .palette = PALETTE_ID_BUTTON,
         .size    = { BIGBUTTON_TILE_W, BIGBUTTON_TILE_H },
         .data    = sButtonTiles,
      };
      //
      {
         const struct VUITileButton_InitParams params = {
            .grid = {
               .pos  = { CTXGRID_BUTTON_OK_X, CTXGRID_BUTTON_OK_Y },
               .size = { CTXGRID_BUTTON_OK_W, CTXGRID_BUTTON_OK_H },
            },
            .callbacks = {
               .on_press = OnButtonOK,
            },
            .labels  = {
               .text     = sButtonLabel_OK,
               .button   = sButtonMapping_OK,
               .colors   = { 4, 6, 7 },
               .y_text   = 4,
               .y_button = 14,
            },
            .screen_pos = { BIGBUTTON_TILE_X, BIGBUTTON_TILE_Y },
            .tiles      = gfx,
            .first_window_tile_id = V_TILE_ID(windows.button_ok),
         };
         auto widget = &sMenuState->vui.widgets.button_ok;
         VUITileButton_Construct(widget, &params);
      }
      {
         const struct VUITileButton_InitParams params = {
            .grid = {
               .pos  = { CTXGRID_BUTTON_BACK_X, CTXGRID_BUTTON_BACK_Y },
               .size = { CTXGRID_BUTTON_BACK_W, CTXGRID_BUTTON_BACK_H },
            },
            .callbacks = {
               .on_press = OnButtonBackspace,
            },
            .labels  = {
               .text     = sButtonLabel_Backspace,
               .button   = sButtonMapping_Backspace,
               .colors   = { 4, 6, 7 },
               .y_text   = 4,
               .y_button = 14,
            },
            .screen_pos = { BIGBUTTON_TILE_X, BIGBUTTON_TILE_Y + BIGBUTTON_TILE_H + 1 },
            .tiles      = gfx,
            .first_window_tile_id = V_TILE_ID(windows.button_backspace),
         };
         auto widget = &sMenuState->vui.widgets.button_backspace;
         VUITileButton_Construct(widget, &params);
      }
   }
   SetUpCharsetLabels();
}
static void Task_WaitFadeIn(u8 task_id) {
   if (!gPaletteFade.active) {
      VUIKeyboardValue* widget = &sMenuState->vui.widgets.value;
      VUIKeyboardValue_SetUnderscoreVisibility(widget, TRUE);
      VUIKeyboardValue_ShowValue              (widget, sMenuState->buffer);
      
      gTasks[task_id].func = Task_OnFrame;
   }
   AnimateBackdrop();
}
static void Task_OnFrame(u8 task_id) {
   AnimateBackdrop();
   
   VUICustomKeyboard* keyboard = &sMenuState->vui.widgets.keyboard;
   if (JOY_NEW(START_BUTTON)) {
      VUIContext_FocusWidget(&sMenuState->vui.context, (VUIWidget*)&sMenuState->vui.widgets.button_ok);
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
   } else if (JOY_NEW(R_BUTTON)) {
      VUICustomKeyboard_NextCharset(keyboard);
   } else {
      VUIContext_HandleInput(&sMenuState->vui.context);
      if (!sMenuState || sMenuState->task_id == TASK_NONE) {
         //
         // User triggered menu exit.
         //
         return;
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
         
         u8 palette_id = IndexOfSpritePaletteTag(SPRITE_PAL_TAG_CHARSET_LABEL);
         if (palette_id != 0xFF) {
            const u16 base_bgcolor = OBJ_PLTT_ID(palette_id) + 8;
            const u16 base_fgcolor = OBJ_PLTT_ID(palette_id) + 2;
            for(u8 i = 0; i < 5; ++i) {
               VUIWidget* button = (VUIWidget*)&sMenuState->vui.widgets.charset_buttons.list[i];
               if (keyboard->charset == i) {
                  gPlttBufferFaded[base_bgcolor + i] = SELECTED_BACK;
               } else {
                  gPlttBufferFaded[base_bgcolor + i] = RGB_WHITE;
               }
               if (sMenuState->vui.context.focused == (VUIWidget*)button) {
                  gPlttBufferFaded[base_fgcolor + i] = RGB_RED;
               } else {
                  gPlttBufferFaded[base_fgcolor + i] = gPlttBufferFaded[base_bgcolor + i];
               }
            }
         }
      }
   }
}
static void Task_BeginExit(u8 task_id) {
   //
   // TODO: If we've been asked to show a dialog box on exit, then 
   // here wait for it to be dismissed.
   //
   BeginNormalPaletteFade(PALETTES_ALL, 1, 0, 16, RGB_BLACK);
   gTasks[sMenuState->task_id].func = Task_WaitFadeOut;
}
static void Task_WaitFadeOut(u8 task_id) {
   AnimateBackdrop();
   
   if (gPaletteFade.active) {
      return;
   }
   
   void(*callback)(const u8*) = sMenuState->callback;
   //
   u8 local_value[VUIKEYBOARDVALUE_MAX_SUPPORTED_SIZE + 1];
   memset(local_value, EOS, sizeof(local_value));
   StringCopy(local_value, sMenuState->buffer);
   Teardown();
   //
   (callback)(local_value);
}
static void Teardown(void) {
   DebugPrintf("[LuNamingScreen][Teardown] Tearing down...");
   if (sMenuState) {
      DebugPrintf("[LuNamingScreen][Teardown] Unmapping BG tilemap buffers...");
      UnsetBgTilemapBuffer(0);
      UnsetBgTilemapBuffer(1);
      UnsetBgTilemapBuffer(2);
      UnsetBgTilemapBuffer(3);
      
      DebugPrintf("[LuNamingScreen][Teardown] Destroying task...");
      DestroyTask(sMenuState->task_id);
      DebugPrintf("[LuNamingScreen][Teardown] Destroying widgets...");
      vui_context_foreach(&sMenuState->vui.context, widget) {
         DebugPrintf("[LuNamingScreen][Teardown] Destroying a widget...");
         if (widget)
            VUIWidget_Destroy(widget);
      }
      
      DebugPrintf("[LuNamingScreen][Teardown] Destroying owned sprites...");
      for(u8 i = 0; i < ARRAY_COUNT(sMenuState->sprite_ids.all); ++i) {
         u8 id = sMenuState->sprite_ids.all[i];
         if (id != SPRITE_NONE) {
            DestroySprite(&gSprites[id]);
            sMenuState->sprite_ids.all[i] = SPRITE_NONE;
         }
      }
      
      DebugPrintf("[LuNamingScreen][Teardown] Destroying owned windows...");
      for(u8 i = 0; i < ARRAY_COUNT(sMenuState->window_ids.all); ++i) {
         u8 id = sMenuState->window_ids.all[i];
         if (id != WINDOW_NONE) {
            RemoveWindow(id);
            sMenuState->window_ids.all[i] = WINDOW_NONE;
         }
      }
      
      DebugPrintf("[LuNamingScreen][Teardown] Freeing menu state...");
      Free(sMenuState);
      sMenuState = NULL;
   }
   DebugPrintf("[LuNamingScreen][Teardown] Freeing window buffers...");
   FreeAllWindowBuffers();
   DebugPrintf("[LuNamingScreen][Teardown] Done.");
}


static bool8 IsNicknamingPokemon(void) {
   return sMenuState->icon.type == LU_NAMINGSCREEN_ICONTYPE_POKEMON;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void OnTextEntryChanged(const u8* value) {
   VUIKeyboardValue_ShowValue(&sMenuState->vui.widgets.value, value);
}
static void OnTextEntryFull(void) {
   PlaySE(SE_FAILURE);
}
static void OnButtonCharset_Upper(void) {
   VUICustomKeyboard_SetCharset(&sMenuState->vui.widgets.keyboard, 0);
}
static void OnButtonCharset_Lower(void) {
   VUICustomKeyboard_SetCharset(&sMenuState->vui.widgets.keyboard, 1);
}
static void OnButtonCharset_Symbol(void) {
   VUICustomKeyboard_SetCharset(&sMenuState->vui.widgets.keyboard, 2);
}
static void OnButtonCharset_AccentUpper(void) {
   VUICustomKeyboard_SetCharset(&sMenuState->vui.widgets.keyboard, 3);
}
static void OnButtonCharset_AccentLower(void) {
   VUICustomKeyboard_SetCharset(&sMenuState->vui.widgets.keyboard, 4);
}
static void OnButtonOK(void) {
   //
   // TODO: If we've been asked to show a dialog box on exit, then 
   // here display it.
   //
   gTasks[sMenuState->task_id].func = Task_BeginExit;
}
static void OnButtonBackspace(void) {
   VUICustomKeyboard_Backspace(&sMenuState->vui.widgets.keyboard);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void InitCB2(void) {
   switch (gMain.state) {
      default:
      case 0:
         gMain.state++;
         break;
      case 1:
         ShowBg(BGLAYER_BACKDROP);
         ShowBg(BGLAYER_CONTENT);
         SetBgTilemapBuffer(BGLAYER_BACKDROP, sMenuState->tilemap_buffers[BGLAYER_BACKDROP]);
         SetBgTilemapBuffer(BGLAYER_CONTENT,  sMenuState->tilemap_buffers[BGLAYER_CONTENT]);
         
         SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
         
         gMain.state++;
         break;
       case 2:
         //ResetTasks(); // we're called from a task-based menu
         
         V_LOAD_TILES(BGLAYER_CONTENT,  blank_tile,   sBlankBGTile);
         V_LOAD_TILES(BGLAYER_CONTENT,  common_tiles, sBGTiles);
         V_LOAD_TILES(BGLAYER_CONTENT,  button_tiles, sButtonTiles);
         DrawBackdrop();
         
         gMain.state++;
         break;
       case 3:
         LuUI_LoadPlayerWindowFrame(
            BGLAYER_CONTENT, // BG layer
            1,               // palette
            V_TILE_ID(user_window_frame)
         );
         gMain.state++;
         break;
       case 4:
         //LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(BACKGROUND_PALETTE_ID_MENU), sizeof(sOptionMenuBg_Pal));
         gMain.state++;
         break;
       case 5:
         LoadPalette(sBackdropPalette,        BG_PLTT_ID(PALETTE_ID_BACKDROP), sizeof(sBackdropPalette));
         LoadPalette(sBGPalette,              BG_PLTT_ID(PALETTE_ID_CHROME),   sizeof(sBGPalette));
         LoadPalette(sButtonPalette,          BG_PLTT_ID(PALETTE_ID_BUTTON),   sizeof(sButtonPalette));
         LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(PALETTE_ID_TEXT),     PLTT_SIZE_4BPP);
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
         PaintTitleBarTiles();
         PaintTitleText();
         SetUpGenderIcon();
         SetUpIcon();
         DrawChromeBorderAround(
            KEYBOARD_TILE_X,
            KEYBOARD_TILE_Y,
            KEYBOARD_TILE_INNER_W + 2,
            KEYBOARD_TILE_INNER_H + 2
         );
         DrawChromeBorderAround(
            VALUE_TILE_X,
            VALUE_TILE_Y,
            sMenuState->vui.widgets.value.max_length + 2,
            2 + 2
         );
         {  // Charset button bar background
            const u8 y = DISPLAY_TILE_HEIGHT - 3;
            
            FillBgTilemapBufferRect(BGLAYER_CONTENT, COMMONTILE_CHARSETBAR_TOP, 0, y + 0, DISPLAY_TILE_WIDTH, 1, PALETTE_ID_CHROME);
            FillBgTilemapBufferRect(BGLAYER_CONTENT, COMMONTILE_CHARSETBAR_MID, 0, y + 1, DISPLAY_TILE_WIDTH, 1, PALETTE_ID_CHROME);
            FillBgTilemapBufferRect(BGLAYER_CONTENT, COMMONTILE_CHARSETBAR_BOT, 0, y + 2, DISPLAY_TILE_WIDTH, 1, PALETTE_ID_CHROME);
         }
         VUITileButton_Repaint(&sMenuState->vui.widgets.button_ok,        FALSE);
         VUITileButton_Repaint(&sMenuState->vui.widgets.button_backspace, FALSE);
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

// -----------------------------------------------------------------------

static const u8 sGenderColors[2][3] = {
   { 1, TEXT_COLOR_LIGHT_BLUE, TEXT_COLOR_BLUE },
   { 1, TEXT_COLOR_LIGHT_RED,  TEXT_COLOR_RED  }
};
static void SetUpGenderIcon() {
   if (sMenuState->gender == MON_GENDERLESS)
      return;
   if (sMenuState->icon.type != LU_NAMINGSCREEN_ICONTYPE_POKEMON)
      //
      // The gender value may be present for OWs; the main use case there is 
      // when starting a new game. When the player picks a gender, we pop a 
      // naming screen, and we need it to show the gender the player just 
      // chose, not whatever gender is leftover in SaveBlock2.
      //
      return;
   
   u8 window_id = sMenuState->window_ids.gender;
   if (window_id == WINDOW_NONE) {
      const struct WindowTemplate tmpl = {
         .bg          = BGLAYER_CONTENT,
         .tilemapLeft = GENDER_WINDOW_TILE_X,
         .tilemapTop  = GENDER_WINDOW_TILE_Y,
         .width       = GENDER_WINDOW_TILE_W,
         .height      = GENDER_WINDOW_TILE_H,
         .paletteNum  = PALETTE_ID_TEXT,
         .baseBlock   = V_TILE_ID(windows.gender)
      };
      
      window_id = sMenuState->window_ids.gender = AddWindow(&tmpl);
      if (window_id == WINDOW_NONE)
         return;
      PutWindowTilemap(window_id);
   }
   
   const u8* text = gText_MaleSymbol;
   u8 color_idx = 0;
   if (sMenuState->gender == MON_FEMALE) {
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

static void PaintTitleBarTiles(void) {
   FillBgTilemapBufferRect(
      BGLAYER_CONTENT,
      COMMONTILE_TITLEBAR_BACK,
      0, 0,
      4, 4,
      PALETTE_ID_CHROME
   );
   FillBgTilemapBufferRect(
      BGLAYER_CONTENT,
      COMMONTILE_TITLEBAR_BACK,
      4, 0,
      1, 3,
      PALETTE_ID_CHROME
   );
   FillBgTilemapBufferRect(
      BGLAYER_CONTENT,
      COMMONTILE_TITLEBAR_BACK,
      5, 0,
      DISPLAY_TILE_WIDTH - 5, 2,
      PALETTE_ID_CHROME
   );
   
   FillBgTilemapBufferRect(
      BGLAYER_CONTENT,
      COMMONTILE_TITLEBAR_EDGE_BOTTOM_A,
      0, 4,
      4, 1,
      PALETTE_ID_CHROME
   );
   FillBgTilemapBufferRect(
      BGLAYER_CONTENT,
      COMMONTILE_TITLEBAR_EDGE_BOTTOM_B,
      5, 2,
      DISPLAY_TILE_WIDTH - 5, 1,
      PALETTE_ID_CHROME
   );
   
   V_SET_TILE(BGLAYER_CONTENT, COMMONTILE_TITLEBAR_DIAGONAL_1, 4, 4, PALETTE_ID_CHROME);
   V_SET_TILE(BGLAYER_CONTENT, COMMONTILE_TITLEBAR_DIAGONAL_2, 4, 3, PALETTE_ID_CHROME);
   V_SET_TILE(BGLAYER_CONTENT, COMMONTILE_TITLEBAR_DIAGONAL_3, 5, 3, PALETTE_ID_CHROME);
   V_SET_TILE(BGLAYER_CONTENT, COMMONTILE_TITLEBAR_DIAGONAL_4, 5, 2, PALETTE_ID_CHROME);
}
//
static const u8 sDefaultTitle[] = _("Enter a name.");static const u8 sNicknameTitle[] = _("{STR_VAR_1}'s nickname?");
//
static void PaintTitleText(void) {
   u8 window_id = sMenuState->window_ids.title;
   if (window_id == WINDOW_NONE) {
      const struct WindowTemplate tmpl = {
         .bg          = BGLAYER_CONTENT,
         .tilemapLeft = TITLE_WINDOW_TILE_X,
         .tilemapTop  = 0,
         .width       = TITLE_WINDOW_TILE_WIDTH,
         .height      = TITLE_WINDOW_TILE_HEIGHT,
         .paletteNum  = PALETTE_ID_TEXT,
         .baseBlock   = V_TILE_ID(windows.title)
      };
      
      window_id = sMenuState->window_ids.title = AddWindow(&tmpl);
      if (window_id == WINDOW_NONE)
         return;
      PutWindowTilemap(window_id);
   }
   
   FillWindowPixelBuffer(window_id, PIXEL_FILL(1));
   
   u8 buffer[32];
   const u8* text = sMenuState->title;
   if (text == NULL) {
      text = sDefaultTitle;
      if (IsNicknamingPokemon()) {
         buffer[0] = EOS;
         StringCopy(buffer, gSpeciesNames[sMenuState->icon.pokemon.species]);
         StringAppend(buffer, gText_PkmnsNickname);
         text = buffer;
      }
   }
   
   u8 colors[3] = { 1, 2, 3 };
   //
   AddTextPrinterParameterized3(
      window_id,
      FONT_BOLD,
      4,
      2,
      colors,
      TEXT_SKIP_DRAW,
      text
   );
   
   CopyWindowToVram(window_id, COPYWIN_FULL);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static const struct OamData sOam_8x8;

static const u32 sCharsetLabelGfx[] = INCBIN_U32("graphics/lu/naming_screen/charset-buttons.4bpp");
static const u16 sCharsetLabelPal[] = INCBIN_U16("graphics/lu/naming_screen/charset-buttons.gbapal");

static const struct SpritePalette sSpritePalettes[] = {
    { sCharsetLabelPal, SPRITE_PAL_TAG_CHARSET_LABEL },
    {}
};
static const struct SpriteSheet sSpriteSheets[] = {
   { sCharsetLabelGfx, sizeof(sCharsetLabelGfx), SPRITE_GFX_TAG_CHARSET_LABEL },
   {},
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Helper for defining a rectangular button consisting of four subsprites:
//  - upper-left corner and upper edge (32x8px)
//  - lower-left corner and lower edge (32x8px)
//  - upper-right corner (either 8x8px or 16x8px)
//  - lower-right corner (either 8x8px or 16x8px)
#define SPRITE_TILE_WIDTH (232 / TILE_WIDTH)
#define SUBSPRITE_GROUP(_x, _w) \
   {                                    \
      .x          = 0,                  \
      .y          = 0,                  \
      .shape      = SPRITE_SHAPE(32x8), \
      .size       = SPRITE_SIZE(32x8),  \
      .tileOffset = (_x) / TILE_WIDTH,  \
      .priority   = 1,                  \
   },                                   \
   {                                    \
      .x          = 0,                  \
      .y          = 8,                  \
      .shape      = SPRITE_SHAPE(32x8), \
      .size       = SPRITE_SIZE(32x8),  \
      .tileOffset = SPRITE_TILE_WIDTH + (_x) / TILE_WIDTH,  \
      .priority   = 1,                  \
   },                                   \
   {                                    \
      .x          = 32,                 \
      .y          =  0,                 \
      .shape      = (_w) - 32 <= 8 ? SPRITE_SHAPE(8x8) : SPRITE_SHAPE(16x8), \
      .size       = (_w) - 32 <= 8 ? SPRITE_SIZE(8x8)  : SPRITE_SIZE(16x8),  \
      .tileOffset = (_x / TILE_WIDTH) + 4, \
      .priority   = 1,                  \
   },                                   \
   {                                    \
      .x          = 32,                 \
      .y          =  8,                 \
      .shape      = (_w) - 32 <= 8 ? SPRITE_SHAPE(8x8) : SPRITE_SHAPE(16x8), \
      .size       = (_w) - 32 <= 8 ? SPRITE_SIZE(8x8)  : SPRITE_SIZE(16x8),  \
      .tileOffset = SPRITE_TILE_WIDTH + (_x / TILE_WIDTH) + 4, \
      .priority   = 1,                  \
   },
   
static const struct Subsprite sSubsprites_CharsetLabel_Upper[] = {
   SUBSPRITE_GROUP(0, 40)
};
static const struct Subsprite sSubsprites_CharsetLabel_Lower[] = {
   SUBSPRITE_GROUP(40, 40)
};
static const struct Subsprite sSubsprites_CharsetLabel_Symbol[] = {
   SUBSPRITE_GROUP(80, 48)
};
static const struct Subsprite sSubsprites_CharsetLabel_AccentUpper[] = {
   SUBSPRITE_GROUP(128, 48)
};
static const struct Subsprite sSubsprites_CharsetLabel_AccentLower[] = {
   SUBSPRITE_GROUP(176, 40)
};
static const struct Subsprite sSubsprites_CharsetLabel_Separator[] = {
   {
      .x          = 0,
      .y          = 0,
      .shape      = SPRITE_SHAPE(8x8),
      .size       = SPRITE_SIZE(8x8),
      .tileOffset = (216 / TILE_WIDTH),
      .priority   = 1,
   },
};
static const struct Subsprite sSubsprites_CharsetLabel_ButtonL[] = {
   {
      .x          = 0,
      .y          = 0,
      .shape      = SPRITE_SHAPE(8x8),
      .size       = SPRITE_SIZE(8x8),
      .tileOffset = SPRITE_TILE_WIDTH + (216 / TILE_WIDTH),
      .priority   = 1,
   },
};
static const struct Subsprite sSubsprites_CharsetLabel_ButtonR[] = {
   {
      .x          = 0,
      .y          = 0,
      .shape      = SPRITE_SHAPE(8x8),
      .size       = SPRITE_SIZE(8x8),
      .tileOffset = SPRITE_TILE_WIDTH + (216 / TILE_WIDTH) + 1,
      .priority   = 1,
   },
};

#undef SPRITE_TILE_WIDTH
#undef SUBSPRITE_GROUP

#define MAKE_SUBSPRITE_TABLE(Name) \
   static const struct SubspriteTable sSubspriteTable_CharsetLabel_##Name[] = {       \
      {ARRAY_COUNT(sSubsprites_CharsetLabel_##Name), sSubsprites_CharsetLabel_##Name} \
   };
MAKE_SUBSPRITE_TABLE(Upper);
MAKE_SUBSPRITE_TABLE(Lower);
MAKE_SUBSPRITE_TABLE(Symbol);
MAKE_SUBSPRITE_TABLE(AccentUpper);
MAKE_SUBSPRITE_TABLE(AccentLower);
MAKE_SUBSPRITE_TABLE(Separator);
MAKE_SUBSPRITE_TABLE(ButtonL);
MAKE_SUBSPRITE_TABLE(ButtonR);
#undef MAKE_SUBSPRITE_TABLE

static const struct SpriteTemplate sSpriteTemplate_CharsetLabel = {
    .tileTag     = SPRITE_GFX_TAG_CHARSET_LABEL,
    .paletteTag  = SPRITE_PAL_TAG_CHARSET_LABEL,
    .oam         = &sOam_8x8,
    .anims       = gDummySpriteAnimTable,
    .images      = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback    = SpriteCallbackDummy
};

static void SetUpCharsetLabels(void) {
   struct VUISpriteButton_InitParams widget_init_params = {
      .callbacks = {
         .on_press = NULL,
      },
      .grid = {
         .pos  = { 0, CTXGRID_CHARSETBUTTON_Y },
         .size = { 1, 1 },
      },
   };
   
   auto widgets = &sMenuState->vui.widgets;
   
   {  // Charset button: Upper
      auto id     = CreateSprite(&sSpriteTemplate_CharsetLabel, 14, 140, 0);
      auto sprite = &gSprites[id];
      auto widget = &sMenuState->vui.widgets.charset_buttons.upper;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Upper);
      
      widget_init_params.grid.pos.x = CTXGRID_CHARSETBUTTON_UPPER_X;
      widget_init_params.callbacks.on_press = OnButtonCharset_Upper;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, id);
   }
   {  // Charset button: Lower
      auto id     = CreateSprite(&sSpriteTemplate_CharsetLabel, 54, 140, 0);
      auto sprite = &gSprites[id];
      auto widget = &sMenuState->vui.widgets.charset_buttons.lower;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Lower);
      
      widget_init_params.grid.pos.x = CTXGRID_CHARSETBUTTON_LOWER_X;
      widget_init_params.callbacks.on_press = OnButtonCharset_Lower;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, id);
   }
   {  // Charset button: Symbol
      auto id     = CreateSprite(&sSpriteTemplate_CharsetLabel, 93, 140, 0);
      auto sprite = &gSprites[id];
      auto widget = &sMenuState->vui.widgets.charset_buttons.symbol;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Symbol);
      
      widget_init_params.grid.pos.x = CTXGRID_CHARSETBUTTON_SYMBOL_X;
      widget_init_params.callbacks.on_press = OnButtonCharset_Symbol;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, id);
   }
   {  // Charset button: Accented Upper
      auto id     = CreateSprite(&sSpriteTemplate_CharsetLabel, 140, 140, 0);
      auto sprite = &gSprites[id];
      auto widget = &sMenuState->vui.widgets.charset_buttons.accent_u;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_AccentUpper);
      
      widget_init_params.grid.pos.x = CTXGRID_CHARSETBUTTON_ACCENTUPPER_X;
      widget_init_params.callbacks.on_press = OnButtonCharset_AccentUpper;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, id);
   }
   {  // Charset button: Accented Lower
      auto id     = CreateSprite(&sSpriteTemplate_CharsetLabel, 185, 140, 0);
      auto sprite = &gSprites[id];
      auto widget = &sMenuState->vui.widgets.charset_buttons.accent_l;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_AccentLower);
      
      widget_init_params.grid.pos.x = CTXGRID_CHARSETBUTTON_ACCENTLOWER_X;
      widget_init_params.callbacks.on_press = OnButtonCharset_AccentLower;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, id);
   }
   
   {  // Separators
      u8 x_positions[4] = { 51, 90, 136, 182 };
      for(u8 i = 0; i < 4; ++i) {
         u8   id     = CreateSprite(&sSpriteTemplate_CharsetLabel, x_positions[i], 146, 0);
         auto sprite = &gSprites[id];
         SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Separator);
         sMenuState->sprite_ids.charset_label_separators[i] = id;
      }
   }
   {  // L-button icon
      u8   id     = CreateSprite(&sSpriteTemplate_CharsetLabel, 5, 144, 0);
      auto sprite = &gSprites[id];
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_ButtonL);
      sMenuState->sprite_ids.charset_labels_button_l = id;
   }
   {  // R-button icon
      u8   id     = CreateSprite(&sSpriteTemplate_CharsetLabel, 227, 144, 0);
      auto sprite = &gSprites[id];
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_ButtonR);
      sMenuState->sprite_ids.charset_labels_button_r = id;
   }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static const struct SpritePalette  sSpritePalette_PCIcon;
static const struct SpriteTemplate sSpriteTemplate_PCIcon;
static const struct SubspriteTable sSubspriteTable_PCIcon[];

#include "constants/event_object_movement.h" // ANIM_STD_GO_SOUTH
#include "global.fieldmap.h" // PLAYER_AVATAR_STATE_NORMAL
#include "event_object_movement.h" // CreateObjectGraphicsSprite
#include "field_player_avatar.h" // GetRivalAvatarGraphicsIdByStateIdAndGender
#include "pokemon_icon.h" // CreateMonIcon

static void SetUpIcon(void) {
   enum {
      PRIORITY = 0,
   };
   
   auto icon = &sMenuState->icon;
   if (icon->type == LU_NAMINGSCREEN_ICONTYPE_NONE)
      return;
   
   switch (icon->type) { // Handle presets.
      case LU_NAMINGSCREEN_ICONTYPE_PC:
         icon->type = LU_NAMINGSCREEN_ICONTYPE_CUSTOM;
         icon->custom.palette     = &sSpritePalette_PCIcon;
         icon->custom.template   = &sSpriteTemplate_PCIcon;
         icon->custom.subsprites = sSubspriteTable_PCIcon;
         icon->custom.offset_x   = 0;
         icon->custom.offset_y   = 1;
         break;
      case LU_NAMINGSCREEN_ICONTYPE_PLAYER:
         icon->type = LU_NAMINGSCREEN_ICONTYPE_OVERWORLD;
         {
            u8 gender = sMenuState->gender;
            if (gender == MON_GENDERLESS) {
               gender = gSaveBlock2Ptr->playerGender;
            }
            icon->overworld.id = GetRivalAvatarGraphicsIdByStateIdAndGender(
               PLAYER_AVATAR_STATE_NORMAL,
               gender
            );
         }
         break;
   }
   
   const u8 type = icon->type;
   
   if (type == LU_NAMINGSCREEN_ICONTYPE_CUSTOM) {
      auto params = &icon->custom;
      AGB_ASSERT(params->palette  != NULL);
      AGB_ASSERT(params->template != NULL);
      if (params->palette && params->template) {
         AGB_WARNING(params->palette->tag == params->template->paletteTag);
         AGB_WARNING(params->palette->tag != TAG_NONE && params->template->paletteTag != TAG_NONE);
      }
      
      LoadSpritePalette(params->palette);
      u8 sprite_id = CreateSprite(
         params->template,
         ICON_BASE_CX + params->offset_x,
         ICON_BASE_CY + params->offset_y,
         0
      );
      if (sprite_id != SPRITE_NONE) {
         auto sprite = &gSprites[sprite_id];
         if (params->subsprites)
            SetSubspriteTables(sprite, params->subsprites);
         sprite->oam.priority = PRIORITY;
      }
      return;
   }
   
   if (type == LU_NAMINGSCREEN_ICONTYPE_POKEMON) {
      LoadMonIconPalettes();
      u8 sprite_id = CreateMonIcon(
         sMenuState->icon.pokemon.species,
         SpriteCallbackDummy,
         ICON_BASE_CX + ICON_OFFSET_PKMN_X,
         ICON_BASE_CY + ICON_OFFSET_PKMN_Y,
         0,
         sMenuState->icon.pokemon.personality,
         1
      );
      if (sprite_id != SPRITE_NONE) {
         auto sprite = &gSprites[sprite_id];
         sprite->oam.priority = PRIORITY;
      }
      return;
   }
   
   if (type == LU_NAMINGSCREEN_ICONTYPE_OVERWORLD) {
      u8 sprite_id = CreateObjectGraphicsSprite(
         sMenuState->icon.overworld.id,
         SpriteCallbackDummy,
         ICON_BASE_CX + ICON_OFFSET_OW_X,
         ICON_BASE_CY + ICON_OFFSET_OW_Y,
         0
      );
      if (sprite_id != SPRITE_NONE) {
         auto sprite = &gSprites[sprite_id];
         sprite->oam.priority = PRIORITY;
         StartSpriteAnim(sprite, ANIM_STD_GO_SOUTH);
      }
      return;
   }
   
   AGB_WARNING(0 && "Unhandled LuNamingScreenIcon type!");
}

static const u8 sPCIconOff_Gfx[] = INCBIN_U8("graphics/naming_screen/pc_icon_off.4bpp");
static const u8 sPCIconOn_Gfx[]  = INCBIN_U8("graphics/naming_screen/pc_icon_on.4bpp");
//
static const union AnimCmd sAnim_PCIcon[] = {
   ANIMCMD_FRAME(0, 2),
   ANIMCMD_FRAME(1, 2),
   ANIMCMD_JUMP(0)
};
static const union AnimCmd* const sAnims_PCIcon[] = {
   sAnim_PCIcon
};
static const struct SpriteFrameImage sImageTable_PCIcon[] = {
   { sPCIconOff_Gfx, sizeof(sPCIconOff_Gfx) },
   { sPCIconOn_Gfx,  sizeof(sPCIconOn_Gfx)  },
};
static const struct SpriteTemplate sSpriteTemplate_PCIcon = {
   .tileTag     = TAG_NONE,
   .paletteTag  = SPRITE_PAL_TAG_CUSTOM_ICON,
   .oam         = &sOam_8x8,
   .anims       = sAnims_PCIcon,
   .images      = sImageTable_PCIcon,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback    = SpriteCallbackDummy
};

/*
[0_]    16x24
[1+] <--Origin
[2_]
*/
static const struct Subsprite sSubsprites_PCIcon[] = {
   {
      .x          = -8,
      .y          = -12,
      .shape      = SPRITE_SHAPE(16x8),
      .size       = SPRITE_SIZE(16x8),
      .tileOffset = 0,
      .priority   = 3
   },
   {
      .x          = -8,
      .y          = -4,
      .shape      = SPRITE_SHAPE(16x8),
      .size       = SPRITE_SIZE(16x8),
      .tileOffset = 2,
      .priority   = 3
   },
   {
      .x          = -8,
      .y          =  4,
      .shape      = SPRITE_SHAPE(16x8),
      .size       = SPRITE_SIZE(16x8),
      .tileOffset = 4,
      .priority   = 3
   }
};
static const struct SubspriteTable sSubspriteTable_PCIcon[] = {
   { ARRAY_COUNT(sSubsprites_PCIcon), sSubsprites_PCIcon }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static const struct OamData sOam_8x8 = {
   .y          = 0,
   .affineMode = ST_OAM_AFFINE_OFF,
   .objMode    = ST_OAM_OBJ_NORMAL,
   .bpp        = ST_OAM_4BPP,
   .shape      = SPRITE_SHAPE(8x8),
   .x          = 0,
   .size       = SPRITE_SIZE(8x8),
   .tileNum    = 0,
   .priority   = 0,
   .paletteNum = 0,
};

// -----------------------------------------------------------------------

static const u32 sBackdropTileset[] = INCBIN_U32("graphics/lu/naming_screen/backdrop-jewel-tiles.4bpp.lz");
static const u32 sBackdropTilemap[] = INCBIN_U32("graphics/lu/naming_screen/backdrop-jewel-tiles.bin.lz");
static const struct BGTilemapInfo sBackdropTilemapInfo = {
   .data = {
      .content = sBackdropTilemap,
      .is_compressed    = TRUE,
      .first_palette_id = 0,
      .first_tile_id    = 0,
   },
   .size = {
      .w = 8,
      .h = 6
   },
};
static void DrawBackdrop(void) {
   V_LOAD_COMPRESSED(backdrop, sBackdropTileset);
   DrawTiledBackground(
      &sBackdropTilemapInfo,
      BGLAYER_BACKDROP,
      PALETTE_ID_BACKDROP,
      V_TILE_ID(backdrop),
      0,
      0
   );
}
static void AnimateBackdrop(void) {
   const u8 BACKDROP_PX_W = sBackdropTilemapInfo.size.w * TILE_WIDTH;
   
   sMenuState->timer++;
   SetGpuReg(REG_OFFSET_BG0HOFS, ((sMenuState->timer / 4) % BACKDROP_PX_W));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void DrawChromeBorderAround(u8 tile_x, u8 tile_y, u8 tile_w, u8 tile_h) {
   #define PAINT(_tile, _x, _y, _w, _h, _fliph, _flipv) \
      FillBgTilemapBufferRect(BGLAYER_CONTENT, (_tile) | ((_fliph) << 10) | ((_flipv) << 11), (_x), (_y), (_w), (_h), PALETTE_ID_CHROME);
   
   // Upper corners
   PAINT(COMMONTILE_CHROME_CORNER_UPPER_A, tile_x,              tile_y,     1, 1, FALSE, FALSE);
   PAINT(COMMONTILE_CHROME_CORNER_UPPER_B, tile_x,              tile_y + 1, 1, 1, FALSE, FALSE);
   PAINT(COMMONTILE_CHROME_CORNER_UPPER_A, tile_x + tile_w - 1, tile_y,     1, 1, TRUE,  FALSE);
   PAINT(COMMONTILE_CHROME_CORNER_UPPER_B, tile_x + tile_w - 1, tile_y + 1, 1, 1, TRUE,  FALSE);
   
   // Lower corners
   PAINT(COMMONTILE_CHROME_CORNER_LOWER_A, tile_x,              tile_y + tile_h - 2, 1, 1, FALSE, FALSE);
   PAINT(COMMONTILE_CHROME_CORNER_LOWER_B, tile_x,              tile_y + tile_h - 1, 1, 1, FALSE, FALSE);
   PAINT(COMMONTILE_CHROME_CORNER_LOWER_A, tile_x + tile_w - 1, tile_y + tile_h - 2, 1, 1, TRUE,  FALSE);
   PAINT(COMMONTILE_CHROME_CORNER_LOWER_B, tile_x + tile_w - 1, tile_y + tile_h - 1, 1, 1, TRUE,  FALSE);
   
   if (tile_w > 2) {
      PAINT(COMMONTILE_CHROME_EDGE_TOP, tile_x + 1, tile_y,              tile_w - 2, 1, FALSE, FALSE);
      PAINT(COMMONTILE_CHROME_EDGE_BOT, tile_x + 1, tile_y + tile_h - 1, tile_w - 2, 1, FALSE, FALSE);
   }
   if (tile_h > 4) {
      PAINT(COMMONTILE_CHROME_EDGE_L, tile_x,              tile_y + 2, 1, tile_h - 4, FALSE, FALSE);
      PAINT(COMMONTILE_CHROME_EDGE_L, tile_x + tile_w - 1, tile_y + 2, 1, tile_h - 4, TRUE,  FALSE);
   }
   
   #undef PAINT
}