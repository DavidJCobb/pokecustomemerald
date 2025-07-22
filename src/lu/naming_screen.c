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
#include "lu/c.h"
#include "lu/ui_helpers.h"
#include "lu/vram_layout_helpers_new.h"

/*//

   TODO:
   
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
   
    - Create a variation on `Keyboard` that allows the including context to specify 
      how many key pages there are, and what keys are on each page.
      
       - We want five pages for this particular screen: uppercase letters; lowercase 
         letters; symbols; accented uppercase letters; accented lowercase letters.
   
    - Figure out why the five charset buttons below the keyboard can't be navigated 
      to.
      
       - Probably wraparound code within the keyboard itself. We'd have to adjust 
         that to do horizontal wraparound rather than vertical.
         
       - Actually, a better option may be to modify VUIContext so that a widget can 
         perform wraparound navigation to itself, if it spans the full length of 
         the context grid on the axis we're navigating along. We would of course 
         want to have a per-widget flag indicating whether the widget itself wants 
         wraparound-to-self behavior.
   
    - Remove the "next charset" button, keeping the dedicated per-charset buttons.
    
    - Implement L_BUTTON and R_BUTTON mappings for switching between charsets 
      without having to move to the buttons.
   
    - Create either sprites or tile-basde graphics for the "OK" and "Backspace" 
      buttons to the right of the keyboard. I'm thinking we should do tile-based 
      graphics, with the label on the upper half of the button, and a TextPrinter 
      used to print the mapped button (Start or B) below that label.
   
    - Implement having the menu accept and store an optional pointer to a title 
      string, and displaying that title string up top. If no title string is given, 
      we should use a default.
      
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

//*/

static const struct SpriteSheet   sSpriteSheets[];
static const struct SpritePalette sSpritePalettes[];
static const u8  sBlankBGTile[]     = INCBIN_U8("graphics/lu/cgo_menu/bg-tile-blank.4bpp"); // color 1
static const u8  sBGTiles[]         = INCBIN_U8("graphics/lu/naming_screen/bg.4bpp");
static const u16 sBGPalette[]       = INCBIN_U16("graphics/lu/naming_screen/bg.gbapal");
static const u8  sBackdropTiles[]   = INCBIN_U8("graphics/lu/naming_screen/backdrop.4bpp");
static const u16 sBackdropPalette[] = INCBIN_U16("graphics/lu/naming_screen/backdrop.gbapal");
enum {
   BGLAYER_BACKDROP = 0,
   BGLAYER_CONTENT  = 1,
   
   PALETTE_ID_CHROME   =  0,
   PALETTE_ID_BACKDROP = 14,
   PALETTE_ID_TEXT     = 15,
   
   SPRITE_GFX_TAG_CHARSET_LABEL = 0x9000,
   SPRITE_PAL_TAG_CHARSET_LABEL = 0x9000,
};
vram_bg_layout {
   vram_bg_tilemap tilemaps[4];
   
   vram_bg_tile blank_tile;
   vram_bg_tile common_tiles[sizeof(sBGTiles) / TILE_SIZE_4BPP];
   vram_bg_tile keyboard_borders[8];
   vram_bg_tile keyboard_body[VUIKEYBOARD_WINDOW_TILE_COUNT];
   vram_bg_tile user_window_frame[9];
   vram_bg_tile keyboard_value[VUIKEYBOARDVALUE_WINDOW_TILE_COUNT];
   vram_bg_tile backdrop[4*4];
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
   struct {
      VUIContext context;
      struct {
         VUIKeyboard      keyboard;
         VUIKeyboardValue value;
         VUISpriteButton  button_ok;
         VUISpriteButton  button_backspace;
         VUISpriteButton  button_charset;
         struct {
            VUISpriteButton upper;
            VUISpriteButton lower;
            VUISpriteButton symbol;
            VUISpriteButton accent_u;
            VUISpriteButton accent_l;
         } charset_buttons;
      } widgets;
      VUIWidget* widget_list[10];
   } vui;
   u8 charset_label_separator_sprite_ids[4];
   u8 charset_label_button_l_sprite_id;
   u8 charset_label_button_r_sprite_id;
   
   u8 tilemap_buffers[4][BG_SCREEN_SIZE];
   u8 timer;
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
static void OnButtonCharset(void);
static void OnButtonOK(void);

static void InitCB2(void);
static void MainCB2(void);
static void VBlankCB(void);

static void SetUpBackdrop(void);
static void AnimateBackdrop(void);
static void PaintTitleBarTiles(void);
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

static void InitState(const struct LuNamingScreenParams* params) {
   AGB_ASSERT(!sMenuState);
   sMenuState = AllocZeroed(sizeof(struct MenuState));
   
   // Why the actual hell do we even need to do this? The BG library should 
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
      list[4] = (VUIWidget*)&widgets->button_charset;
      
      list[5] = (VUIWidget*)&widgets->charset_buttons.upper;
      list[6] = (VUIWidget*)&widgets->charset_buttons.lower;
      list[7] = (VUIWidget*)&widgets->charset_buttons.symbol;
      list[8] = (VUIWidget*)&widgets->charset_buttons.accent_u;
      list[9] = (VUIWidget*)&widgets->charset_buttons.accent_l;
   }
   
   LoadSpriteSheets(sSpriteSheets);
   LoadSpritePalettes(sSpritePalettes);
   
   {
      VUIContext* context = &sMenuState->vui.context;
      context->widgets.list = sMenuState->vui.widget_list;
      context->widgets.size = 10;
      context->w = 6;
      context->h = 4;
      context->allow_wraparound_x = context->allow_wraparound_y = TRUE;
   }
   {
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
   {
      VUIKeyboard* widget = &sMenuState->vui.widgets.keyboard;
      const struct VUIKeyboard_InitParams params = {
         .buffer = {
            .data = sMenuState->buffer,
            .size = max_length,
         },
         .callbacks = {
            .on_text_changed      = OnTextEntryChanged,
            .on_text_at_maxlength = OnTextEntryFull,
         },
         .grid = {
            .pos  = { 0, 0 },
            .size = { 5, 3 },
         },
         .bg_layer      = BGLAYER_CONTENT,
         .palette       = PALETTE_ID_TEXT,
         .colors        = text_colors,
         .tile_x        = 8,
         .tile_y        = 7,
         .first_tile_id = V_TILE_ID(keyboard_body),
      };
      VUIKeyboard_Construct(widget, &params);
   }
   {
      VUISpriteButton* widget = &sMenuState->vui.widgets.button_ok;
      const struct VUISpriteButton_InitParams params = {
         .callbacks = {
            .on_press = OnButtonOK,
         },
         .grid = {
            .pos  = { 6, 0 },
            .size = { 1, 1 },
         },
      };
      VUISpriteButton_Construct(widget, &params);
   }
   {
      VUISpriteButton* widget = &sMenuState->vui.widgets.button_backspace;
      const struct VUISpriteButton_InitParams params = {
         .callbacks = {
            .on_press = NULL,
         },
         .grid = {
            .pos  = { 6, 1 },
            .size = { 1, 1 },
         },
      };
      VUISpriteButton_Construct(widget, &params);
   }
   {
      VUISpriteButton* widget = &sMenuState->vui.widgets.button_charset;
      const struct VUISpriteButton_InitParams params = {
         .callbacks = {
            .on_press = NULL,
         },
         .grid = {
            .pos  = { 6, 2 },
            .size = { 1, 1 },
         },
      };
      VUISpriteButton_Construct(widget, &params);
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
      
      UnsetBgTilemapBuffer(0);
      UnsetBgTilemapBuffer(1);
      UnsetBgTilemapBuffer(2);
      UnsetBgTilemapBuffer(3);
      
      DebugPrintf("[LuNamingScreen][Teardown] Destroying task...");
      DestroyTask(sMenuState->task_id);
      DebugPrintf("[LuNamingScreen][Teardown] Destroying all widgets...");
      vui_context_foreach(&sMenuState->vui.context, widget) {
         DebugPrintf("[LuNamingScreen][Teardown] Destroying widget %08X...", widget);
         if (widget)
            VUIWidget_Destroy(widget);
      }
      
      DebugPrintf("[LuNamingScreen][Teardown] Destroying charset label separator sprites...");
      for(u8 i = 0; i < ARRAY_COUNT(sMenuState->charset_label_separator_sprite_ids); ++i) {
         u8 id = sMenuState->charset_label_separator_sprite_ids[id];
         if (id != SPRITE_NONE) {
            DestroySprite(&gSprites[id]);
            sMenuState->charset_label_separator_sprite_ids[id] = SPRITE_NONE;
         }
      }
      if (sMenuState->charset_label_button_l_sprite_id != SPRITE_NONE) {
         DestroySprite(&gSprites[sMenuState->charset_label_button_l_sprite_id]);
         sMenuState->charset_label_button_l_sprite_id = SPRITE_NONE;
      }
      if (sMenuState->charset_label_button_r_sprite_id != SPRITE_NONE) {
         DestroySprite(&gSprites[sMenuState->charset_label_button_r_sprite_id]);
         sMenuState->charset_label_button_r_sprite_id = SPRITE_NONE;
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
static void OnButtonCharset_Upper(void) {
   VUIKeyboard_SetCharset(&sMenuState->vui.widgets.keyboard, 0);
}
static void OnButtonCharset_Lower(void) {
   VUIKeyboard_SetCharset(&sMenuState->vui.widgets.keyboard, 1);
}
static void OnButtonCharset_Symbol(void) {
   VUIKeyboard_SetCharset(&sMenuState->vui.widgets.keyboard, 2);
}
static void OnButtonCharset_AccentUpper(void) {
   VUIKeyboard_SetCharset(&sMenuState->vui.widgets.keyboard, 3);
}
static void OnButtonCharset_AccentLower(void) {
   VUIKeyboard_SetCharset(&sMenuState->vui.widgets.keyboard, 4);
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
         //LoadPalette(sOptionsListingPalette, BG_PLTT_ID(BACKGROUND_PALETTE_ID_TEXT), sizeof(sOptionsListingPalette));
         LoadPalette(sBackdropPalette,        BG_PLTT_ID(PALETTE_ID_BACKDROP), sizeof(sBackdropPalette));
         LoadPalette(sBGPalette,              BG_PLTT_ID(PALETTE_ID_CHROME),   sizeof(sBGPalette));
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
         {  // Charset button bar background
            const u16 top = V_TILE_ID(common_tiles[0]);
            const u16 mid = V_TILE_ID(common_tiles[4]);
            const u16 bot = V_TILE_ID(common_tiles[8]);
            
            const u8 y = DISPLAY_TILE_HEIGHT - 3;
            
            FillBgTilemapBufferRect(BGLAYER_CONTENT, top, 0, y + 0, DISPLAY_TILE_WIDTH, 1, PALETTE_ID_CHROME);
            FillBgTilemapBufferRect(BGLAYER_CONTENT, mid, 0, y + 1, DISPLAY_TILE_WIDTH, 1, PALETTE_ID_CHROME);
            FillBgTilemapBufferRect(BGLAYER_CONTENT, bot, 0, y + 2, DISPLAY_TILE_WIDTH, 1, PALETTE_ID_CHROME);
            
         }
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
   
   /*// DISABLED: THIS LOOKS BAD.
   //// A possible alternative would be to have a second layer moving overtop 
   //// of it, parallax-like, with blending.
   //
   // Gemstone shine.
   //
   {
      u16* colors_src = &gPlttBufferUnfaded[BG_PLTT_ID(PALETTE_ID_BACKDROP) + 1];
      u16* colors_dst = &gPlttBufferFaded  [BG_PLTT_ID(PALETTE_ID_BACKDROP) + 1];
      for(u8 i = 0; i < 3; ++i) {
         colors_dst[i] = colors_src[i];
         auto dst = (struct PlttData*) &colors_dst[i];
         dst->r -= 1;
         dst->g -= 1;
         dst->b -= 1;
      }
      
      auto shadow    = (struct PlttData*) &colors_dst[0];
      auto midtone   = (struct PlttData*) &colors_dst[1];
      auto highlight = (struct PlttData*) &colors_dst[2];
      
      const u8 shine_starts[] = { 30, 60 };
      const u8 shine_length   = 12;
      
      u8 shine_step = sMenuState->timer % 90;
      for(u8 i = 0; i < sizeof(shine_starts); ++i) {
         if (shine_step + shine_length < shine_starts[i]) {
            u8 elapsed = shine_step - shine_starts[i];
            if (elapsed < 3) {
               highlight->g += 1;
            } else if (elapsed < 6) {
               midtone->r   += 1;
               midtone->g   -= 1;
               midtone->b   += 1;
            } else if (elapsed < 9) {
               shadow->r    += 1;
               shadow->g    -= 1;
            } else {
               shadow->r    += 1;
               shadow->b    += 1;
               highlight->g += 1;
            }
            break;
         }
      }
   }
   //*/
}

static void PaintTitleBarTiles(void) {
   const u16 fill_tile   = V_TILE_ID(common_tiles[2]);
   const u16 edge_tile_a = V_TILE_ID(common_tiles[9]);
   const u16 edge_tile_b = V_TILE_ID(common_tiles[11]);
   FillBgTilemapBufferRect(
      BGLAYER_CONTENT,
      fill_tile,
      0, 0,
      4, 4,
      PALETTE_ID_CHROME
   );
   FillBgTilemapBufferRect(
      BGLAYER_CONTENT,
      fill_tile,
      4, 0,
      1, 3,
      PALETTE_ID_CHROME
   );
   FillBgTilemapBufferRect(
      BGLAYER_CONTENT,
      fill_tile,
      5, 0,
      DISPLAY_TILE_WIDTH - 5, 2,
      PALETTE_ID_CHROME
   );
   
   FillBgTilemapBufferRect(
      BGLAYER_CONTENT,
      edge_tile_a,
      0, 4,
      4, 1,
      PALETTE_ID_CHROME
   );
   FillBgTilemapBufferRect(
      BGLAYER_CONTENT,
      edge_tile_b,
      5, 2,
      DISPLAY_TILE_WIDTH - 5, 1,
      PALETTE_ID_CHROME
   );
   
   V_SET_TILE(BGLAYER_CONTENT, V_TILE_ID(common_tiles[10]), 4, 4, PALETTE_ID_CHROME);
   V_SET_TILE(BGLAYER_CONTENT, V_TILE_ID(common_tiles[ 6]), 4, 3, PALETTE_ID_CHROME);
   V_SET_TILE(BGLAYER_CONTENT, V_TILE_ID(common_tiles[ 7]), 5, 3, PALETTE_ID_CHROME);
   V_SET_TILE(BGLAYER_CONTENT, V_TILE_ID(common_tiles[ 3]), 5, 2, PALETTE_ID_CHROME);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static const struct OamData sOam_8x8;

static const u32 gCharsetLabelGfx[] = INCBIN_U32("graphics/lu/naming_screen/charset-labels.4bpp");
static const u16 gCharsetLabelPal[] = INCBIN_U16("graphics/lu/naming_screen/charset-labels.gbapal");

static const struct SpritePalette sSpritePalettes[] = {
    { gCharsetLabelPal, SPRITE_PAL_TAG_CHARSET_LABEL },
    {}
};
static const struct SpriteSheet sSpriteSheets[] = {
   { gCharsetLabelGfx, sizeof(gCharsetLabelGfx), SPRITE_GFX_TAG_CHARSET_LABEL },
   {},
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static const struct Subsprite sSubsprites_CharsetLabel_Upper[] = {
   {
      .x          = 0,
      .y          = 0,
      .shape      = SPRITE_SHAPE(32x8),
      .size       = SPRITE_SIZE(32x8),
      .tileOffset = 0,
      .priority   = 1,
   },
};
static const struct Subsprite sSubsprites_CharsetLabel_Lower[] = {
   {
      .x          = 0,
      .y          = 0,
      .shape      = SPRITE_SHAPE(32x8),
      .size       = SPRITE_SIZE(32x8),
      .tileOffset = 4,
      .priority   = 1,
   },
};
static const struct Subsprite sSubsprites_CharsetLabel_Symbol[] = {
   {
      .x          = 0,
      .y          = 0,
      .shape      = SPRITE_SHAPE(32x8),
      .size       = SPRITE_SIZE(32x8),
      .tileOffset = 8,
      .priority   = 1,
   },
   {
      .x          = 32,
      .y          = 0,
      .shape      = SPRITE_SHAPE(8x8),
      .size       = SPRITE_SIZE(8x8),
      .tileOffset = 12,
      .priority   = 1,
   },
};
static const struct Subsprite sSubsprites_CharsetLabel_AccentUpper[] = {
   {
      .x          = 0,
      .y          = 0,
      .shape      = SPRITE_SHAPE(32x8),
      .size       = SPRITE_SIZE(32x8),
      .tileOffset = 13,
      .priority   = 1,
   },
   {
      .x          = 32,
      .y          = 0,
      .shape      = SPRITE_SHAPE(8x8),
      .size       = SPRITE_SIZE(8x8),
      .tileOffset = 17,
      .priority   = 1,
   },
};
static const struct Subsprite sSubsprites_CharsetLabel_AccentLower[] = {
   {
      .x          = 0,
      .y          = 0,
      .shape      = SPRITE_SHAPE(32x8),
      .size       = SPRITE_SIZE(32x8),
      .tileOffset = 18,
      .priority   = 1,
   },
};
static const struct Subsprite sSubsprites_CharsetLabel_Separator[] = {
   {
      .x          = 0,
      .y          = 0,
      .shape      = SPRITE_SHAPE(8x8),
      .size       = SPRITE_SIZE(8x8),
      .tileOffset = 22,
      .priority   = 1,
   },
};
static const struct Subsprite sSubsprites_CharsetLabel_ButtonL[] = {
   {
      .x          = 0,
      .y          = 0,
      .shape      = SPRITE_SHAPE(8x8),
      .size       = SPRITE_SIZE(8x8),
      .tileOffset = 23,
      .priority   = 1,
   },
};
static const struct Subsprite sSubsprites_CharsetLabel_ButtonR[] = {
   {
      .x          = 0,
      .y          = 0,
      .shape      = SPRITE_SHAPE(8x8),
      .size       = SPRITE_SIZE(8x8),
      .tileOffset = 24,
      .priority   = 1,
   },
};
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
         .pos  = { 0, 3 },
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
   
   sprite_ids.upper = CreateSprite(&sSpriteTemplate_CharsetLabel, 18, 144, 0);
   {
      auto sprite = &gSprites[sprite_ids.upper];
      auto widget = &sMenuState->vui.widgets.charset_buttons.upper;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Upper);
      
      widget_init_params.grid.pos.x = 0;
      widget_init_params.callbacks.on_press = OnButtonCharset_Upper;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, sprite_ids.upper);
   }
   
   sprite_ids.lower = CreateSprite(&sSpriteTemplate_CharsetLabel, 58, 144, 0);
   {
      auto sprite = &gSprites[sprite_ids.lower];
      auto widget = &sMenuState->vui.widgets.charset_buttons.lower;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Lower);
      
      widget_init_params.grid.pos.x = 1;
      widget_init_params.callbacks.on_press = OnButtonCharset_Lower;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, sprite_ids.lower);
   }
   
   sprite_ids.symbol = CreateSprite(&sSpriteTemplate_CharsetLabel, 97, 144, 0);
   {
      auto sprite = &gSprites[sprite_ids.symbol];
      auto widget = &sMenuState->vui.widgets.charset_buttons.symbol;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_Symbol);
      
      widget_init_params.grid.pos.x = 2;
      widget_init_params.callbacks.on_press = OnButtonCharset_Symbol;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, sprite_ids.symbol);
   }
   
   sprite_ids.accent_u = CreateSprite(&sSpriteTemplate_CharsetLabel, 144, 144, 0);
   {
      auto sprite = &gSprites[sprite_ids.accent_u];
      auto widget = &sMenuState->vui.widgets.charset_buttons.accent_u;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_AccentUpper);
      
      widget_init_params.grid.pos.x = 3;
      widget_init_params.callbacks.on_press = OnButtonCharset_AccentUpper;
      VUISpriteButton_Construct(widget, &widget_init_params);
      VUISpriteButton_TakeSprite(widget, sprite_ids.accent_u);
   }
   
   sprite_ids.accent_l = CreateSprite(&sSpriteTemplate_CharsetLabel, 189, 144, 0);
   {
      auto sprite = &gSprites[sprite_ids.accent_l];
      auto widget = &sMenuState->vui.widgets.charset_buttons.accent_l;
      SetSubspriteTables(sprite, sSubspriteTable_CharsetLabel_AccentLower);
      
      widget_init_params.grid.pos.x = 4;
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