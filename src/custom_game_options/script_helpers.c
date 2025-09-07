#include "custom_game_options/script_helpers.h"
#include "custom_game_options/options.h"

bool8 GetCustomGameOptionBool(u8 id) {
   switch (id) {
      case CGOPTION_BOOL_GRANT_CATCH_XP:
         return gCustomGameOptions.battle.catching.enable_catch_exp;
   }
   return FALSE;
}

bool8 GetCustomGameStateBool(u8 id) {
   return FALSE;
}