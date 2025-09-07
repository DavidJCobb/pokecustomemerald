#include "field_specials/ShouldDistributeEonTicket.h"
#include "global.h" // *sigh*
#include "constants/flags.h" // FLAG_...
#include "constants/vars.h" // VAR_DISTRIBUTE_EON_TICKET
#include "event_data.h" // VarGet
#include "item.h" // CheckBagHasItem
#include "custom_game_options/options.h"

bool32 ShouldDistributeEonTicket(void) {
   switch (gCustomGameOptions.events.eon_ticket) {
      case CGO_EONTICKETMODE_VANILLA:
         if (!VarGet(VAR_DISTRIBUTE_EON_TICKET))
            return FALSE;
         break;
      case CGO_EONTICKETMODE_AFTER_ROAMING_LATI:
         if (!FlagGet(FLAG_DEFEATED_LATIAS_OR_LATIOS) && !FlagGet(FLAG_CAUGHT_LATIAS_OR_LATIOS))
            return FALSE;
         break;
   }
   
   if (CheckBagHasItem(ITEM_EON_TICKET, 1)) // skip if player already has the Eon Ticket
      return FALSE;

   return TRUE;
}