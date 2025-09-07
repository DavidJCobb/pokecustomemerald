#ifndef GUARD_LU_CGO_HANDLERS_BATTLE_H
#define GUARD_LU_CGO_HANDLERS_BATTLE_H
#include "gba/types.h"

u16 ApplyCustomGameBattleAccuracyScaling(u16);
void ApplyCustomGameBattleDamageScaling(void); // modifies gBattleMoveDamage

u32 ApplyCustomGameBattleMoneyVictoryScaling(u32);

bool8 CustomGamesAllowBattleBackfieldHealing();
bool8 CustomGamesAllowRevivesInBattle();

#endif