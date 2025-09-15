#include "menus/short_string_entry/api.h"
#include "menus/short_string_entry/menu.h"
#include "menus/short_string_entry/params.h"
#include "gba/defines.h" // EWRAM_DATA
#include "main.h" // SetMainCallback2
#include "global.h" // dependency of pokemon.h
#include "data.h" // gSpeciesNames
#include "event_data.h" // FlagGet, VarGet
#include "pokemon.h" // CalculatePlayerPartyCount, GetMonData, GetMonGender; dependency of pokemon_storage_system.h
#include "pokemon_storage_system.h" // BOX_NAME_LENGTH, GetBoxNamePtr
#include "string_util.h" // StringCopy, gStringVar1, ..., gStringVar4
#include "strings.h"
#include "walda_phrase.h" // WALDA_PHRASE_LENGTH
#include "constants/characters.h" // EOS
#include "constants/event_objects.h" // OBJ_EVENT_GFX_MAN_1
#include "constants/global.h" // PLAYER_NAME_LENGTH
#include "constants/flags.h" // FLAG_SYS_PC_LANETTE
#include "constants/vars.h" // VAR_PC_BOX_TO_SEND_MON

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
      .callbacks = {
         .show_message_before_menu_exit = NULL,
         .on_menu_exit                  = CommonCallback,
      },
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
      .callbacks = {
         .show_message_before_menu_exit = NULL,
         .on_menu_exit                  = CommonCallback,
      },
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
      .callbacks = {
         .show_message_before_menu_exit = NULL,
         .on_menu_exit                  = CommonCallback,
      },
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

#include "field_specials.h" // GetPCBoxToSendMon, IsDestinationBoxFull
static const u8* const sTransferredToPCMessages[] = {
   gText_PkmnTransferredSomeonesPC,
   gText_PkmnTransferredLanettesPC,
   gText_PkmnTransferredSomeonesPCBoxFull,
   gText_PkmnTransferredLanettesPCBoxFull
};
static const u8* ShowCaughtPokemonPCDestination(const u8* string) {
   if (CalculatePlayerPartyCount() < PARTY_SIZE) {
      return NULL;
   }
   u8 stringToDisplay = 0;
   if (!IsDestinationBoxFull()) {
      StringCopy(gStringVar1, GetBoxNamePtr(VarGet(VAR_PC_BOX_TO_SEND_MON)));
      StringCopy(gStringVar2, string);
   } else {
      StringCopy(gStringVar1, GetBoxNamePtr(VarGet(VAR_PC_BOX_TO_SEND_MON)));
      StringCopy(gStringVar2, string);
      StringCopy(gStringVar3, GetBoxNamePtr(GetPCBoxToSendMon()));
      stringToDisplay = 2;
   }
   if (FlagGet(FLAG_SYS_PC_LANETTE))
      stringToDisplay++;
   StringExpandPlaceholders(gStringVar4, sTransferredToPCMessages[stringToDisplay]);
   return gStringVar4;
}
extern void ShortStringEntryMenu_FreshlyCaughtPokemon(
   MainCallback    cb2,
   struct Pokemon* pokemon,
   u8*             dst_nickname
) {
   sNamingScreenFollowup.main_callback_2 = cb2;
   sNamingScreenFollowup.destination     = dst_nickname;
   
   u16 species = GetMonData(pokemon, MON_DATA_SPECIES);
   
   StringCopy(gStringVar1, gSpeciesNames[species]);
   StringExpandPlaceholders(gStringVar4, gText_PkmnsNickname);
   
   struct ShortStringEntryMenuParams params = {
      .callbacks = {
         .show_message_before_menu_exit = ShowCaughtPokemonPCDestination,
         .on_menu_exit                  = CommonCallback,
      },
      .initial_value = dst_nickname,
      .max_length    = POKEMON_NAME_LENGTH,
      //
      .has_gender = TRUE,
      .gender     = GetMonGender(pokemon),
      .icon       = {
         .type    = SHORTSTRINGENTRY_ICONTYPE_POKEMON,
         .pokemon = {
            .species     = species,
            .personality = GetMonData(pokemon, MON_DATA_PERSONALITY, NULL)
         },
      },
      .title = gStringVar4,
   };
   OpenShortStringEntryMenu(&params);
}

extern void ShortStringEntryMenu_RenamePokemon(
   MainCallback    cb2,
   struct Pokemon* pokemon,
   u8*             dst_nickname
) {
   ShortStringEntryMenu_RenameBoxPokemon(cb2, &pokemon->box, dst_nickname);
}
extern void ShortStringEntryMenu_RenameBoxPokemon(
   MainCallback       cb2,
   struct BoxPokemon* pokemon,
   u8*                dst_nickname
) {
   sNamingScreenFollowup.main_callback_2 = cb2;
   sNamingScreenFollowup.destination     = dst_nickname;
   
   u16 species = GetBoxMonData(pokemon, MON_DATA_SPECIES);
   
   StringCopy(gStringVar1, gSpeciesNames[species]);
   StringExpandPlaceholders(gStringVar4, gText_PkmnsNickname);
   
   struct ShortStringEntryMenuParams params = {
      .callbacks = {
         .show_message_before_menu_exit = NULL,
         .on_menu_exit                  = CommonCallback,
      },
      .initial_value = dst_nickname,
      .max_length    = POKEMON_NAME_LENGTH,
      //
      .has_gender = TRUE,
      .gender     = GetBoxMonGender(pokemon),
      .icon       = {
         .type    = SHORTSTRINGENTRY_ICONTYPE_POKEMON,
         .pokemon = {
            .species     = species,
            .personality = GetBoxMonData(pokemon, MON_DATA_PERSONALITY, NULL)
         },
      },
      .title = gStringVar4,
   };
   OpenShortStringEntryMenu(&params);
}