#include "battle/battle_allows_forfeiting.h"
#include "global.h"
#include "constants/battle.h"
#include "battle.h"

// The vanilla game allows the player to voluntarily forfeit Battle Frontier and 
// Trainer Hill battles. This function and the battle core have both been edited 
// so that if the player is allowed to forfeit other trainer battles, the game 
// behaves as if the player lost.
bool8 CurrentBattleAllowsForfeiting(void) {
   if (gBattleTypeFlags & BATTLE_TYPE_LINK) {
      return FALSE;
   }
   
   return
      (gBattleTypeFlags & BATTLE_TYPE_TRAINER) != 0
   //&& (gBattleTypeFlags & (BATTLE_TYPE_FRONTIER | BATTLE_TYPE_TRAINER_HILL)) != 0
   ;
}