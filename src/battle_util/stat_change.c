#include "battle_util/stat_change.h"

enum CannotDecreaseAnyStatsReason CheckDecreaseBattlerAnyStats(
   u8    battler,
   u16   move,
   bool8 certain,
   bool8 bypass_protect
) {
   if (move == 0)
      move = gCurrentMove;
   
   if (move != MOVE_CURSE) {
      if (gSideTimers[GET_BATTLER_SIDE(battler)].mistTimer && !certain) {
         return CANNOTDECREASEANYSTATSREASON_MIST;
      }
      if (
         !bypass_protect
      && gProtectStructs[battler].protected
      && (gBattleMoves[move].flags & FLAG_PROTECT_AFFECTED)
      ) {
         return CANNOTDECREASEANYSTATSREASON_PROTECT;
      }
      if (!certain) {
         switch (gBattleMons[battler].ability) {
            case ABILITY_CLEAR_BODY:
            case ABILITY_WHITE_SMOKE:
               return CANNOTDECREASEANYSTATSREASON_ABILITY;
         }
      }
   }
   return CANNOTDECREASEANYSTATSREASON_NONE;
}

enum CannotDecreaseSpecificStatReason CheckDecreaseBattlerStat(
   u8    battler,
   u8    stat,
   bool8 certain
) {
   if (!certain) {
      //
      // Fail if the subject's ability blocks reductions of this specific stat.
      //
      u16   ability = gBattleMons[battler].ability;
      bool8 blocked = FALSE;
      if (ability == ABILITY_KEEN_EYE && stat == STAT_ACC) {
         blocked = TRUE;
      } else if (ability == ABILITY_HYPER_CUTTER && stat == STAT_ATK) {
         blocked = TRUE;
      }
      
      if (blocked) {
         return CANNOTDECREASESPECIFICSTATREASON_ABILITY;
      }
   }
   if (gBattleMons[battler].statStages[currStat] <= MIN_STAT_STAGE) {
      return CANNOTDECREASESPECIFICSTATREASON_BOUNDED;
   }
   return CANNOTDECREASESPECIFICSTATREASON_NONE;
}