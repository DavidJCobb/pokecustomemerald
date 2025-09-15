#include "menus/short_string_entry/state.h"
#include "menus/short_string_entry/menu.h"
#include "lu/c.h"
#include "lu/macros/ARRAY_COUNT.h"
#include "global.h" // for transitive include of <string.h>, for memset
#include "gba/isagbprint.h"
#include "bg.h" // SetBgTilemapBuffer. UnsetBgTilemapBuffer
#include "malloc.h" // AllocZeroed, Free
#include "sprite.h" // DestroySprite
#include "string_util.h" // StringCopy
#include "task.h" // DestroyTask
#include "window.h" // RemoveWindow
#include "constants/characters.h" // EOS
#include "constants/pokemon.h" // MON_GENDERLESS and friends

EWRAM_DATA struct ShortStringEntryMenuState* gShortStringEntryMenuState = NULL;

#define STATE gShortStringEntryMenuState

extern void ShortStringEntryMenu_CreateState(void) {
   AGB_ASSERT(!STATE);
   
   STATE = AllocZeroed(sizeof(struct ShortStringEntryMenuState));
   
   STATE->task_id = TASK_NONE;
   for(u8 i = 0; i < ARRAY_COUNT(STATE->sprite_ids.all); ++i)
      STATE->sprite_ids.all[i] = SPRITE_NONE;
   for(u8 i = 0; i < ARRAY_COUNT(STATE->window_ids.all); ++i)
      STATE->window_ids.all[i] = WINDOW_NONE;
   
   STATE->gender = MON_GENDERLESS;
   
   SetBgTilemapBuffer(0, STATE->tilemap_buffers[0]);
   SetBgTilemapBuffer(1, STATE->tilemap_buffers[1]);
   SetBgTilemapBuffer(2, STATE->tilemap_buffers[2]);
   SetBgTilemapBuffer(3, STATE->tilemap_buffers[3]);
   
   memset(STATE->buffer, EOS, sizeof(STATE->buffer));
   
   // VUI: Set up widget pointers.
   {
      auto list    = STATE->vui.widget_list;
      auto widgets = &STATE->vui.widgets;
      list[0] = (VUIWidget*)&widgets->keyboard;
      list[1] = (VUIWidget*)&widgets->value;
      list[2] = (VUIWidget*)&widgets->button_ok;
      list[3] = (VUIWidget*)&widgets->button_backspace;
      for(u8 i = 0; i < 5; ++i) {
         auto widget = &widgets->charset_buttons.list[i];
         list[4 + i] = (VUIWidget*)widget;
      }
      
      VUIContext* context = &STATE->vui.context;
      context->widgets.list = STATE->vui.widget_list;
      context->widgets.size = ARRAY_COUNT(STATE->vui.widget_list);
      
      VUICustomKeyboard_Construct(&widgets->keyboard);
      VUIKeyboardValue_Construct(&widgets->value);
      VUIButton_Construct(&widgets->button_ok);
      VUIButton_Construct(&widgets->button_backspace);
      for(u8 i = 0; i < 5; ++i) {
         auto widget = &widgets->charset_buttons.list[i];
         VUISpriteButton_Construct(widget);
      }
   }
}

extern void ShortStringEntryMenu_InitState(const struct ShortStringEntryMenuParams* params) {
   AGB_ASSERT(STATE != NULL);
   
   STATE->gender = MON_GENDERLESS;
   if (params->has_gender)
      STATE->gender = params->gender;
   STATE->icon  = params->icon;
   STATE->title = params->title;
   
   u8 max_length = params->max_length;
   AGB_WARNING(max_length <= sizeof(STATE->buffer) - 1);
   if (max_length >= sizeof(STATE->buffer)) {
      max_length = sizeof(STATE->buffer) - 1;
   }
   STATE->max_length = max_length;
   
   STATE->callbacks = params->callbacks;
   if (params->initial_value) {
      u8* end  = StringCopy(STATE->buffer, params->initial_value);
      u16 size = end - STATE->buffer;
      AGB_WARNING(size <= sizeof(STATE->buffer));
   }
}

extern void ShortStringEntryMenu_TeardownGraphics(void) {
   if (!STATE)
      return;
   UnsetBgTilemapBuffer(0);
   UnsetBgTilemapBuffer(1);
   UnsetBgTilemapBuffer(2);
   UnsetBgTilemapBuffer(3);
   vui_context_foreach(&STATE->vui.context, widget) {
DebugPrintf("[%s] Tearing down a widget...", __func__);
DebugPrintf("[%s] Widget is: %08X", __func__, widget);
if (widget) DebugPrintf("[%s] Widget VTBL is: %08X", __func__, widget->functions);
if (widget->functions) DebugPrintf("[%s] Widget destroy v-func is: %08X", __func__, widget->functions->_impl_destroy);
      if (widget)
         VUIWidget_Destroy(widget);
   }
   for(u8 i = 0; i < ARRAY_COUNT(STATE->sprite_ids.all); ++i) {
      u8 id = STATE->sprite_ids.all[i];
      if (id != SPRITE_NONE) {
         DestroySprite(&gSprites[id]);
         STATE->sprite_ids.all[i] = SPRITE_NONE;
      }
   }
   for(u8 i = 0; i < ARRAY_COUNT(STATE->window_ids.all); ++i) {
      u8 id = STATE->window_ids.all[i];
      if (id != WINDOW_NONE) {
         RemoveWindow(id);
         STATE->window_ids.all[i] = WINDOW_NONE;
      }
   }
}

extern void ShortStringEntryMenu_DestroyState(void) {
   if (!STATE)
      return;
   
   if (STATE->task_id != TASK_NONE)
      DestroyTask(STATE->task_id);
   
   ShortStringEntryMenu_TeardownGraphics();
   
   Free(STATE);
   STATE = NULL;
}