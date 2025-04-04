#include "lu/widgets/keybind_strip.h"
#include "constants/characters.h"
#include "menu.h" // AddTextPrinterParameterized3
#include "palette.h" // LoadPalette
#include "text.h" // DrawKeypadIcon, GetKeypadIconWidth, GetStringWidth
#include "text_window.h" // GetTextWindowPalette
#include "window.h"

extern void InitKeybindStrip(
   struct LuKeybindStrip* widget,
   const struct LuKeybindStripInitParams* params
) {
   widget->entries         = NULL;
   widget->entry_count     = 0;
   widget->enabled_entries = 0;
   widget->_window_id      = WINDOW_NONE;
   
   LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(params->palette_id), PLTT_SIZE_4BPP);
   
   const struct WindowTemplate tmpl = {
      .bg          = params->bg_layer,
      .tilemapLeft = 0,
      .tilemapTop  = DISPLAY_TILE_HEIGHT - KEYBIND_STRIP_TILE_HEIGHT,
      .width       = KEYBIND_STRIP_TILE_WIDTH,
      .height      = KEYBIND_STRIP_TILE_HEIGHT,
      .paletteNum  = params->palette_id,
      .baseBlock   = params->first_tile_id
   };
   u8 window_id = AddWindow(&tmpl);
   widget->_window_id = window_id;
   PutWindowTilemap(window_id);
   
   FillWindowPixelBuffer(window_id, PIXEL_FILL(15));
   CopyWindowToVram(window_id, COPYWIN_FULL);
}

extern void SetKeybindStripAllEntriesEnabled(struct LuKeybindStrip* widget, bool8 enabled) {
   if (enabled) {
      if (widget->entry_count >= 8) {
         widget->enabled_entries = 0xFF;
      } else {
         widget->enabled_entries = (1 << widget->entry_count) - 1;
      }
   } else {
      widget->enabled_entries = 0;
   }
}
extern void SetKeybindStripEntryEnabled(struct LuKeybindStrip* widget, u8 index, bool8 enabled) {
   if (enabled) {
      widget->enabled_entries |= (1 << index);
   } else {
      widget->enabled_entries &= ~(1 << index);
   }
}

static const u8 sString_Space[] = _(" ");

extern void RepaintKeybindStrip(const struct LuKeybindStrip* widget) {
   const u8 color[3] = { TEXT_DYNAMIC_COLOR_6, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY };
   const u8 win      = widget->_window_id;
   
   u8 text_x = 2;
   const u8 text_y = 1;
   
   FillWindowPixelBuffer(win, PIXEL_FILL(15));
   
   for(int i = 0; i < widget->entry_count; ++i) {
      if (!(widget->enabled_entries & (1 << i)))
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
      }
      AddTextPrinterParameterized3(win, FONT_SMALL, text_x, text_y, color, TEXT_SKIP_DRAW, label);
      text_x += GetStringWidth(FONT_SMALL, label, 0);
      text_x += GetStringWidth(FONT_SMALL, sString_Space, 0);
   }
   CopyWindowToVram(win, COPYWIN_FULL);
}

extern void DestroyKeybindStrip(struct LuKeybindStrip* widget) {
   RemoveWindow(widget->_window_id);
   widget->_window_id = WINDOW_NONE;
}