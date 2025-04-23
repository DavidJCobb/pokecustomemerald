#include "field_specials/ShouldDistributeEonTicket.h"
#include "constants/vars.h"
#include "event_data.h" // VarGet
#include "item.h" // CheckBagHasItem
#include "lu/custom_game_options.h"

bool32 ShouldDistributeEonTicket(void) {
   if (!VarGet(VAR_DISTRIBUTE_EON_TICKET) && !gCustomGameOptions.events.eon_ticket)
      return FALSE;
    
   if (CheckBagHasItem(ITEM_EON_TICKET, 1)) // skip if player already has the Eon Ticket
      return FALSE;

   return TRUE;
}