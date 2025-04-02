#include "lu/widgets/keybind_strip.h"
#include "constants/characters.h"
#include "menu.h" // AddTextPrinterParameterized3
#include "palette.h" // LoadPalette
#include "text.h" // DrawKeypadIcon, GetKeypadIconWidth, GetStringWidth
#include "window.h"

void InitKeybindStrip(struct LuKeybindStrip* widget) {
   LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(widget->config.palette_id), PLTT_SIZE_4BPP);
   
   const struct WindowTemplate tmpl = {
      .bg          = widget->config.bg_layer,
      .tilemapLeft = 0,
      .tilemapTop  = DISPLAY_TILE_HEIGHT - KEYBIND_STRIP_TILE_HEIGHT,
      .width       = KEYBIND_STRIP_TILE_WIDTH,
      .height      = KEYBIND_STRIP_TILE_HEIGHT,
      .paletteNum  = widget->config.palette_id,
      .baseBlock   = widget->config.first_tile_id
   };
   widget->state.window_id = AddWindow(&tmpl);
   PutWindowTilemap(widget->state.window_id);
   
   FillWindowPixelBuffer(win, PIXEL_FILL(15));
   CopyWindowToVram(win, COPYWIN_FULL);
}

const u8* sSpaceString = _(" ");

void RepaintKeybindStrip(struct LuKeybindStrip* widget) {
   const u8 color[3] = { TEXT_DYNAMIC_COLOR_6, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY };
   const u8 win      = widget->state.window_id;
   
   u8 text_x = 2;
   const u8 text_y = 1;
   
   FillWindowPixelBuffer(win, PIXEL_FILL(15));
   
   for(int i = 0; i < KEYBIND_STRIP_ENTRY_COUNT; ++i) {
      if (!widget->entries[i].show)
         continue;
      const u8* label = widget->entries[i].text;
      if (!label)
         continue;
      
      const u16 buttons = widget->entries[i].buttons;
      if (buttons) {
         for(int j = 0; j <= CHAR_DPAD_NONE; ++j) {
            u16 mask = 1 << j;
            if (buttons & mask) {
               DrawKeypadIcon(win, j, text_x, text_y);
               text_x += GetKeypadIconWidth(j);
               
               // 90% of the time, a keybind strip entry will use just a 
               // single button, so early-out in those cases once we find 
               // that button.
               if (buttons == mask) {
                  break;
               }
            }
         }
         text_x += GetStringWidth(FONT_SMALL, sSpaceString, 0);
      }
      AddTextPrinterParameterized3(win, FONT_SMALL, text_x, text_y, color, TEXT_SKIP_DRAW, label);
      text_x += GetStringWidth(FONT_SMALL, label, 0);
   }
   CopyWindowToVram(win, COPYWIN_FULL);
}

void DestroyKeybindStrip(struct LuKeybindStrip* widget) {
   RemoveWindow(widget->state.window_id);
   widget->state.window_id = WINDOW_NONE;
}