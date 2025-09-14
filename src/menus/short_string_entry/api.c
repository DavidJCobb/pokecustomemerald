#include "menus/short_string_entry/api.h"
#include "gba/defines.h" // EWRAM_DATA
#include "lu/naming_screen.h"
#include "main.h" // SetMainCallback2
#include "global.h" // dependency of pokemon.h
#include "pokemon.h" // dependency of pokemon_storage_system.h
#include "pokemon_storage_system.h" // BOX_NAME_LENGTH
#include "string_util.h" // StringCopy
#include "strings.h"
#include "constants/characters.h" // EOS
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

extern void RenamePCBox(MainCallback cb2, u8* boxName) {
   sNamingScreenFollowup.main_callback_2 = cb2;
   sNamingScreenFollowup.destination     = boxName;
   
   struct LuNamingScreenParams params = {
      .callback      = CommonCallback,
      .initial_value = boxName,
      .max_length    = BOX_NAME_LENGTH,
      //
      .icon  = {
         .type = LU_NAMINGSCREEN_ICONTYPE_PC,
      },
      .title = gText_BoxName,
   };
   LuNamingScreen(&params);
}

extern void RequestPlayerName(MainCallback cb2) {
   sNamingScreenFollowup.main_callback_2 = cb2;
   sNamingScreenFollowup.destination     = gSaveBlock2Ptr->playerName;
   
   struct LuNamingScreenParams params = {
      .callback      = CommonCallback,
      .initial_value = gSaveBlock2Ptr->playerName,
      .max_length    = PLAYER_NAME_LENGTH,
      //
      .icon  = {
         .type = LU_NAMINGSCREEN_ICONTYPE_PLAYER,
      },
      .title = gText_YourName,
   };
   LuNamingScreen(&params);
}