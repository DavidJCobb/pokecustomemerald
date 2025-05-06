
# Battle script command contexts

## `critmessage`

Preconditions:

* `gBattlerAttacker` must have been set up
* `gCritMultiplier` must have been set up
* `gMoveResultFlags` must have been set up (we check `MOVE_RESULT_NO_EFFECT`)

## `resultmessage`

Preconditions:

* `gBattlerAttacker` must have been set up
* `gBattlerTarget` must have been set up
* `gMoveResultFlags` must have been set up
* `gBattleCommunication[MISS_TYPE]` must have been set up





## `switchhandleorder`

This script command is one of the steps performed as part of switching a Pokemon out. It handles reordering a battler's containing party. (Switches are handled as multiple script commands and native code steps, since effects like animations need to play in the desired order and generally will be routed through the battle scripts.)

Args:

* Battler to switch out
* Switch-out behavior
  * **0: Queue simultaneous switch-in.** Check all battlers for a `CONTROLLER_CHOSENMONRETURNVALUE` controller-to-engine message. If one is present, then set `gBattleStruct->monToSwitchIntoId[battler]` as appropriate, and (if its run-once flag in `gBattleStruct` permits) invoke `RecordedBattle_SetBattlerAction`. This behavior is expected to be used after an `openpartymenu` call without the "optional" flag. This behavior is invoked only by `BattleScript_LinkHandleFaintedMonMultiple`.
  * **1: Execute switch-in action.** If this is not a Multi Battle, invoke `SwitchPartyOrder(battler)` immediately. This is called only by `BattleScript_ActionSwitch`.
  * **2: Queue local switch-in.** If its run-once flag in `gBattleStruct` permits, invoke `RecordedBattle_SetBattlerAction`. Either way, fall through to behavior 3. This behavior is expected to be used after an `openpartymenu` call, and is invoked by `BattleScript_EffectBatonPass` and `BattleScript_FaintedMonTryChoose` (for both replacing a fainted Pokemon, and for optionally switching in at the same time that an opponent NPC replaces their fainted Pokemon).
  * **3: Execute queued switch-in.** Check the last-received controller-to-engine message for the current battler, and assume it indicates a desired switch-in. Set `gBattleStruct->monToSwitchIntoId[battler]` as appropriate. Invoke `SwitchPartyOrder` or, if this is a Multi Battle, a suitable alternative. Write `gBattlerAttacker`'s species to `B_BUFF_1` and `battler`'s nickname to `B_BUFF_2`.

The behavior could be given a hypothetical enum:

```c
enum {
   // One or both of the player's Pokemon have fainted during a Link Battle. They've 
   // been asked to pick a replacement to switch-in; accept their answer and queue a 
   // switch-in as appropriate.
   QUEUE_LINK_SWITCH_REACTION,
   
   // The battler has chosen to switch their Pokemon, i.e. B_ACTION_SWITCH. It is 
   // assumed that the switch has already been queued. Execute the switch (i.e. 
   // reorder the battler's party).
   EXECUTE_SWITCH_ACTION,
   
   // The battler is being switched out as the result of a battle script, e.g. Baton 
   // Pass, a Pokemon fainting locally, or the "Shift" Battle Style. That battler's 
   // controller is expected to have chosen a party slot to switch in. Record a "switch" 
   // battler action (i.e. for Recorded Battles), queue the switch, and then fall 
   // through to the next behavior.
   EXECUTE_LOCAL_SWITCH_REACTION,
   
   // The battler's controller is expected to have chosen a party slot to switch in. 
   // Execute that switch (i.e. adjust their party ordering).
   EXECUTE_LAST_RECEIVED_SWITCH
};
```

Usage notes:

* `BattleScript_LinkHandleFaintedMonMultiple` runs `switchhandleorder BS_FAINTED 0` twice, each time after `openpartyscreen`. Then, it performs both switch-ins, beginning by running `switchhandleorder <battler> 3` for each.