#include "menus/short_string_entry/api.h"
#include "menus/short_string_entry/menu.h"
#include "menus/short_string_entry/params.h"
#include "gba/defines.h" // EWRAM_DATA
#include "main.h" // SetMainCallback2
#include "global.h" // dependency of pokemon.h
#include "pokemon.h" // dependency of pokemon_storage_system.h
#include "pokemon_storage_system.h" // BOX_NAME_LENGTH
#include "string_util.h" // StringCopy
#include "strings.h"
#include "walda_phrase.h" // WALDA_PHRASE_LENGTH
#include "constants/characters.h" // EOS
#include "constants/event_objects.h" // OBJ_EVENT_GFX_MAN_1
#include "constants/global.h" // PLAYER_NAME_LENGTH

static EWRAM_DATA struct {
   MainCallback main_callback_2;
   u8*          destination;
} sNamingScreenFollowup = { 0 };

static EWRAM_DATA MainCallback sNamingScreenFollowupCB2 = NULL;

static void CommonCallback(const u8* buffer) {
   if (buffer && buffer[0] != EOS) {
      StringCopy(sNamingScreenFollowup.destination, buffer);
   }
   SetMainCallback2(sNamingScreenFollowup.main_callback_2);
   sNamingScreenFollowup.main_callback_2 = NULL;
}

//

extern void ShortStringEntryMenu_RenamePCBox(MainCallback cb2, u8* boxName) {
   sNamingScreenFollowup.main_callback_2 = cb2;
   sNamingScreenFollowup.destination     = boxName;
   
   struct ShortStringEntryMenuParams params = {
      .callback      = CommonCallback,
      .initial_value = boxName,
      .max_length    = BOX_NAME_LENGTH,
      //
      .icon  = {
         .type = SHORTSTRINGENTRY_ICONTYPE_PC,
      },
      .title = gText_BoxName,
   };
   OpenShortStringEntryMenu(&params);
}

extern void ShortStringEntryMenu_RenamePlayer(MainCallback cb2) {
   sNamingScreenFollowup.main_callback_2 = cb2;
   sNamingScreenFollowup.destination     = gSaveBlock2Ptr->playerName;
   
   struct ShortStringEntryMenuParams params = {
      .callback      = CommonCallback,
      .initial_value = gSaveBlock2Ptr->playerName,
      .max_length    = PLAYER_NAME_LENGTH,
      //
      .icon  = {
         .type = SHORTSTRINGENTRY_ICONTYPE_PLAYER,
      },
      .title = gText_YourName,
   };
   OpenShortStringEntryMenu(&params);
}

extern void ShortStringEntryMenu_WaldasPassword(MainCallback cb2, u8* string) {
   sNamingScreenFollowup.main_callback_2 = cb2;
   sNamingScreenFollowup.destination     = string;
   
   struct ShortStringEntryMenuParams params = {
      .callback      = CommonCallback,
      .initial_value = string,
      .max_length    = WALDA_PHRASE_LENGTH,
      //
      .icon  = {
         .type      = SHORTSTRINGENTRY_ICONTYPE_OVERWORLD,
         .overworld = {
            .id = OBJ_EVENT_GFX_MAN_1,
         },
      },
      .title = gText_TellHimTheWords,
   };
   OpenShortStringEntryMenu(&params);
}