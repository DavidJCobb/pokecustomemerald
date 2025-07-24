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
#include "task.h"
#include "text.h"
#include "text_window.h" // GetTextWindowPalette
#include "window.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "constants/songs.h" // SE_SELECT and other sound effect constants
#include "lu/c.h"
#include "lu/ui_helpers.h"
#include "lu/vram_layout_helpers_new.h"

/*//

   TODO:
      
    - Implement the various cases for which the vanilla naming screen is used.
    
       - Nickname a Pokemon (freshly caught)
       - Nickname a Pokemon (already owned)
       - Change the name of the player-character
       - Change the name of a PC Box
       - Say a password to Walda
       
       = In general, we want to accept and store a union of...
          - Pokemon data
             - Species ID
             - Personality
             - Gender
          - Overworld sprite ID (Walda or the player)
          - Generic sprite params (PC box sprite)
          
          = Maybe we want to allow a gender symbol for non-Pokemon too, e.g. when 
            the player is entering their name?
       
       = When a Pokemon is given, and no title is given, we should print the title 
         as "<SPECIES>'s nickname?"
   
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
static const u8  sBackdropTiles[]   = INCBIN_U8("graphics/lu/naming_screen/backdrop.4bpp");
static const u16 sBackdropPalette[] = INCBIN_U16("graphics/lu/naming_screen/backdrop.gbapal");
static const u8  sButtonTiles[]     = INCBIN_U8("graphics/lu/naming_screen/tile-button.4bpp");
static const u16 sButtonPalette[]   = INCBIN_U16("graphics/lu/naming_screen/tile-button.gbapal");
enum {
   BGLAYER_BACKDROP = 0,
   BGLAYER_CONTENT  = 1,
   
   PALETTE_ID_CHROME   =  0,
   PALETTE_ID_BUTTON   =  1,
   PALETTE_ID_BACKDROP = 14,
   PALETTE_ID_TEXT     = 15,
   
   SPRITE_GFX_TAG_CHARSET_LABEL = 0x9000,
   SPRITE_PAL_TAG_CHARSET_LABEL = 0x9000,
   
   BIGBUTTON_TILE_X = 23,
   BIGBUTTON_TILE_Y =  7,
   BIGBUTTON_TILE_W =  5,
   BIGBUTTON_TILE_H =  4,
   BIGBUTTON_WIN_TILE_COUNT = BIGBUTTON_TILE_W * BIGBUTTON_TILE_H,
   
   TITLE_WINDOW_TILE_X      = 6,
   TITLE_WINDOW_TILE_WIDTH  = DISPLAY_TILE_WIDTH - TITLE_WINDOW_TILE_X - 1,
   TITLE_WINDOW_TILE_HEIGHT = 2,
   TITLE_WINDOW_TILE_COUNT  = TITLE_WINDOW_TILE_WIDTH * TITLE_WINDOW_TILE_HEIGHT,
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
   vram_bg_tile backdrop[4*4];
   vram_bg_tile title_text_window[TITLE_WINDOW_TILE_COUNT];
   vram_bg_tile ok_button_tiles[BIGBUTTON_WIN_TILE_COUNT];
   vram_bg_tile backspace_button_tiles[BIGBUTTON_WIN_TILE_COUNT];
};
__verify_vram_bg_layout;

enum {
   COMMONTILE_CHARSETBAR_TOP = V_TILE_ID(common_tiles) + 0,
   COMMONTILE_BIGBUTTON_CORNER,
   COMMONTILE_TITLEBAR_BACK,
   COMMONTILE_TITLEBAR_DIAGONAL_4,
   
   COMMONTILE_CHARSETBAR_MID,
   COMMONTILE_BIGBUTTON_EDGE_H,
   COMMONTILE_TITLEBAR_DIAGONAL_2,
   COMMONTILE_TITLEBAR_DIAGONAL_3,
   
   COMMONTILE_CHARSETBAR_BOT,
   COMMONTILE_TITLEBAR_EDGE_BOTTOM_A,
   COMMONTILE_TITLEBAR_DIAGONAL_1,
   COMMONTILE_TITLEBAR_EDGE_BOTTOM_B,
   
   COMMONTILE_TRANSPARENT,
   COMMONTILE_BIGBUTTONSEL_CORNER,
   COMMONTILE_BIGBUTTONSEL_EDGE_V,
   COMMONTILE_BIGBUTTONSEL_EDGE_H,
   
   COMMONTILE_BIGBUTTON_EDGE_V,
   COMMONTILE_BIGBUTTON_FILL,
};

// The abstract-grid layout for our VUI context.
enum {
   /*
   
      +--------------+----+
      |              | OK |
      |   KEYBOARD   +----+
      |              | BK |
      +--------------+----+
      | Charsets...  |
      +--------------+
   
   */
   CTXGRID_KEYBOARD_X = 0,
   CTXGRID_KEYBOARD_Y = 0,
   CTXGRID_KEYBOARD_W = 5,
   CTXGRID_KEYBOARD_H = 3,
   
   CTXGRID_BUTTON_OK_X = CTXGRID_KEYBOARD_X + CTXGRID_KEYBOARD_W,
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
   CTXGRID_CHARSETBUTTON_ACCENTLOWER_X = 3,
   CTXGRID_CHARSETBUTTON_ACCENTUPPER_X = 4,
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
   const u8* title; // optional
   u8 title_window_id;
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
   u8 charset_label_separator_sprite_ids[4];
   u8 charset_label_button_l_sprite_id;
   u8 charset_label_button_r_sprite_id;
   
   u8 tilemap_buffers[4][BG_SCREEN_SIZE];
   u8 timer;
   
   u8 ok_button_window_id;
   u8 backspace_button_window_id;
};
static EWRAM_DATA struct MenuState* sMenuState = NULL;

static void InitState(const struct LuNamingScreenParams*);
static void Task_WaitFadeIn(u8);
static void Task_OnFrame(u8);
static void Teardown(void);

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

static void SetUpBackdrop(void);
static void AnimateBackdrop(void);
static void PaintTitleBarTiles(void);
static void PaintTitleText(void);
static void SetUpCharsetLabels(void);

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
   
   sMenuState->title = params->title;
   sMenuState->title_window_id = WINDOW_NONE;
   
   sMenuState->ok_button_window_id = WINDOW_NONE;
   sMenuState->backspace_button_window_id = WINDOW_NONE;
   
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
         .tile_x = 8,
         .tile_y = 3,
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
         .tile_x        = 7,
         .tile_y        = 7,
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
            .first_window_tile_id = V_TILE_ID(ok_button_tiles),
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
            .first_window_tile_id = V_TILE_ID(backspace_button_tiles),
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
      
      DebugPrintf("[LuNamingScreen][Teardown] Destroying separator sprites...");
      for(u8 i = 0; i < ARRAY_COUNT(sMenuState->charset_label_separator_sprite_ids); ++i) {
         u8 id = sMenuState->charset_label_separator_sprite_ids[i];
         if (id != SPRITE_NONE) {
            DestroySprite(&gSprites[id]);
            sMenuState->charset_label_separator_sprite_ids[i] = SPRITE_NONE;
         }
      }
      DebugPrintf("[LuNamingScreen][Teardown] Destroying L-button and R-button sprites...");
      if (sMenuState->charset_label_button_l_sprite_id != SPRITE_NONE) {
         DestroySprite(&gSprites[sMenuState->charset_label_button_l_sprite_id]);
         sMenuState->charset_label_button_l_sprite_id = SPRITE_NONE;
      }
      if (sMenuState->charset_label_button_r_sprite_id != SPRITE_NONE) {
         DestroySprite(&gSprites[sMenuState->charset_label_button_r_sprite_id]);
         sMenuState->charset_label_button_r_sprite_id = SPRITE_NONE;
      }
      
      DebugPrintf("[LuNamingScreen][Teardown] Destroying title text window...");
      if (sMenuState->title_window_id != WINDOW_NONE) {
         RemoveWindow(sMenuState->title_window_id);
         sMenuState->title_window_id = WINDOW_NONE;
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
   void(*callback)(const u8*) = sMenuState->callback;
   
   u8 local_value[VUIKEYBOARDVALUE_MAX_SUPPORTED_SIZE + 1];
   memset(local_value, EOS, sizeof(local_value));
   StringCopy(local_value, sMenuState->buffer);
   Teardown();
   
   (callback)(local_value);
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
         
         V_LOAD_TILES(BGLAYER_BACKDROP, backdrop,     sBackdropTiles);
         V_LOAD_TILES(BGLAYER_CONTENT,  blank_tile,   sBlankBGTile);
         V_LOAD_TILES(BGLAYER_CONTENT,  common_tiles, sBGTiles);
         V_LOAD_TILES(BGLAYER_CONTENT,  button_tiles, sButtonTiles);
         
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
         SetUpBackdrop();
         PaintTitleBarTiles();
         PaintTitleText();
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

static void SetUpBackdrop(void) {
   enum {
      BGLAYER_TILE_WIDTH   = (256 / TILE_WIDTH),
      BGLAYER_TILE_HEIGHT  = (256 / TILE_HEIGHT),
      BACKDROP_TILE_WIDTH  = 4,
      BACKDROP_TILE_HEIGHT = 4,
   };
   for(u8 sx = 0; sx + (BACKDROP_TILE_WIDTH - 1) < BGLAYER_TILE_WIDTH; sx += BACKDROP_TILE_WIDTH) {
      for(u8 sy = 0; sy < BGLAYER_TILE_HEIGHT; ++sy) {
         u8  ty = sy % BACKDROP_TILE_HEIGHT;
         u16 ti = ty * BACKDROP_TILE_WIDTH;
         WriteSequenceToBgTilemapBuffer(
            BGLAYER_BACKDROP,
            V_TILE_ID(backdrop[0]) + ti,
            sx,
            sy,
            BACKDROP_TILE_WIDTH,
            1,
            PALETTE_ID_BACKDROP,
            1
         );
      }
   }
   if (BGLAYER_TILE_WIDTH % BACKDROP_TILE_WIDTH) {
      for(
         u8 sx = (BGLAYER_TILE_WIDTH - (BGLAYER_TILE_WIDTH % BACKDROP_TILE_WIDTH));
         sx < BGLAYER_TILE_WIDTH;
         ++sx
      ) {
         u8 tx = sx % BACKDROP_TILE_WIDTH;
         for(u8 sy = 0; sy < BGLAYER_TILE_HEIGHT; ++sy) {
            u8  ty = sy % BACKDROP_TILE_HEIGHT;
            u16 ti = ty * BACKDROP_TILE_WIDTH + tx;
            V_SET_TILE(
               BGLAYER_BACKDROP,
               V_TILE_ID(backdrop[0]) + ti,
               sx,
               sy,
               PALETTE_ID_BACKDROP
            );
         }
      }
   }
   CopyBgTilemapBufferToVram(BGLAYER_BACKDROP);
}
static void AnimateBackdrop(void) {
   //ChangeBgX(BGLAYER_BACKDROP, -2, BG_COORD_ADD);
   //
   // The above function doesn't seem to actually update the BG layer position; 
   // we get, like, one update per second at max. Is there ANYTHING in the GF 
   // BG library that actually works?!
   //
   sMenuState->timer++;
   SetGpuReg(REG_OFFSET_BG0HOFS, ((sMenuState->timer / 4) % 32));
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
static const u8 sDefaultTitle[] = _("Enter a name.");
//
static void PaintTitleText(void) {
   u8 window_id = sMenuState->title_window_id;
   if (window_id == WINDOW_NONE) {
      const struct WindowTemplate tmpl = {
         .bg          = BGLAYER_CONTENT,
         .tilemapLeft = 6,
         .tilemapTop  = 0,
         .width       = TITLE_WINDOW_TILE_WIDTH,
         .height      = TITLE_WINDOW_TILE_HEIGHT,
         .paletteNum  = PALETTE_ID_TEXT,
         .baseBlock   = V_TILE_ID(title_text_window)
      };
      
      window_id = sMenuState->title_window_id = AddWindow(&tmpl);
      if (window_id == WINDOW_NONE)
         return;
      PutWindowTilemap(window_id);
   }
   
   FillWindowPixelBuffer(window_id, PIXEL_FILL(1));
   
   const u8* text = sMenuState->title;
   if (text == NULL) {
      text = sDefaultTitle;
      //
      // TODO: If nicknaming a Pokemon, generate text of the form 
      //       "<species>'s nickname?"
      //
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
   
   union {
      u8 list[5];
      struct {
         u8 upper;
         u8 lower;
         u8 symbol;
         u8 accent_u;
         u8 accent_l;
      };
   } sprite_ids;
   auto widgets = &sMenuState->vui.widgets;
   
   sprite_ids.upper = CreateSprite(&sSpriteTemplate_CharsetLabel, 14, 140, 0);
   {
      auto sprite = &gSprites[sprite_ids.upper];
      auto widget = &sMenuState->vui.widgets.charset_buttons.upper;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Upper);
      
      widget_init_params.grid.pos.x = CTXGRID_CHARSETBUTTON_UPPER_X;
      widget_init_params.callbacks.on_press = OnButtonCharset_Upper;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, sprite_ids.upper);
   }
   
   sprite_ids.lower = CreateSprite(&sSpriteTemplate_CharsetLabel, 54, 140, 0);
   {
      auto sprite = &gSprites[sprite_ids.lower];
      auto widget = &sMenuState->vui.widgets.charset_buttons.lower;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Lower);
      
      widget_init_params.grid.pos.x = CTXGRID_CHARSETBUTTON_LOWER_X;
      widget_init_params.callbacks.on_press = OnButtonCharset_Lower;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, sprite_ids.lower);
   }
   
   sprite_ids.symbol = CreateSprite(&sSpriteTemplate_CharsetLabel, 93, 140, 0);
   {
      auto sprite = &gSprites[sprite_ids.symbol];
      auto widget = &sMenuState->vui.widgets.charset_buttons.symbol;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Symbol);
      
      widget_init_params.grid.pos.x = CTXGRID_CHARSETBUTTON_SYMBOL_X;
      widget_init_params.callbacks.on_press = OnButtonCharset_Symbol;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, sprite_ids.symbol);
   }
   
   sprite_ids.accent_u = CreateSprite(&sSpriteTemplate_CharsetLabel, 140, 140, 0);
   {
      auto sprite = &gSprites[sprite_ids.accent_u];
      auto widget = &sMenuState->vui.widgets.charset_buttons.accent_u;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_AccentUpper);
      
      widget_init_params.grid.pos.x = CTXGRID_CHARSETBUTTON_ACCENTUPPER_X;
      widget_init_params.callbacks.on_press = OnButtonCharset_AccentUpper;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, sprite_ids.accent_u);
   }
   
   sprite_ids.accent_l = CreateSprite(&sSpriteTemplate_CharsetLabel, 185, 140, 0);
   {
      auto sprite = &gSprites[sprite_ids.accent_l];
      auto widget = &sMenuState->vui.widgets.charset_buttons.accent_l;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_AccentLower);
      
      widget_init_params.grid.pos.x = CTXGRID_CHARSETBUTTON_ACCENTLOWER_X;
      widget_init_params.callbacks.on_press = OnButtonCharset_AccentLower;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, sprite_ids.accent_l);
   }
   
   {
      u8 x_positions[4] = { 51, 90, 136, 182 };
      for(u8 i = 0; i < 4; ++i) {
         u8   id     = CreateSprite(&sSpriteTemplate_CharsetLabel, x_positions[i], 146, 0);
         auto sprite = &gSprites[id];
         SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Separator);
         sMenuState->charset_label_separator_sprite_ids[i] = id;
      }
   }
   {
      u8   id     = CreateSprite(&sSpriteTemplate_CharsetLabel, 5, 144, 0);
      auto sprite = &gSprites[id];
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_ButtonL);
      sMenuState->charset_label_button_l_sprite_id = id;
   }
   {
      u8   id     = CreateSprite(&sSpriteTemplate_CharsetLabel, 227, 144, 0);
      auto sprite = &gSprites[id];
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_ButtonR);
      sMenuState->charset_label_button_r_sprite_id = id;
   }
}

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