[link-action-selection-script][./concepts.md#actions]
[link-battle-tv-hooks](./concepts.md#Battle%20TV)

# Battle-related globals

## Conventions used in this document

### Tags

Several fields have a similar purpose but are located far apart from one another. In these cases, Markdown footnotes are used to associate them together, with the text of these footnotes labeled "**Tag**" to distinguish them from any actual footnotes we may use.

## In brief

| Struct | Purpose |
| :- | :- |
| `DisableStruct` | States per battler, scoped to the battler's presence on the field. |
| `ProtectStruct` | States per battler, scoped to a single turn. |
| `SpecialStatus` | States per battler, scoped to a single action. |
| `SideTimer` | Timers/intensities for status conditions affecting a side of the battlefield (e.g. Mist, Spikes). |
| `WishFutureKnock` | Move states that are persistent. This includes moves like Future Sight and Wish, which queue an event to happen in the future, with the state for that event persisting even if the caster faints or switches out. It includes the state of Knock Off (indicating which Pokemon, identified by battlefield side and party slot, have lost their items to the move). It includes weather-altering moves (tracking the number of turns remaining until the weather condition ends). |
| `AI_ThinkingStruct` |
| `UsedMoves` |
| `BattleHistory` |
| `StatsArray` |
| `BattleResources` |
| `BattleResults` |
| `BattleTv_Side` |
| `BattleTv_Position` |
| `BattleTv_Mon` |
| `BattleTv` |
| `BattleTvMovePoints` |
| `LinkBattleHeader` |
| `BattleStruct` | Everything and the kitchen sink: global state, plus a dumping ground for latent function state. |
| `BattleScripting` | Intended to hold variables that scripts can directly read and write, but also holds a handful of latent state variables used by native code only. |

## Simple globals

### Overarching

| Name | Purpose |
| :- | :- |
| `gAbsentBattlerFlags` | See also: `gBattleStruct->absentBattlerFlags`. |
| `gBattleTypeFlags` | Flags-mask indicating the battle type. Broadly, this tells you whether this is a trainer/wild battle, whether it's a singleplayer/link battle, whether it's a single/double/multi battle, and whether it's in a battle facility. |
| `gBattlersCount` | Number of battlers currently on the field. |
| `gBattleEnvironment` | Formerly called `gBattleTerrain`. Indicates the battle terrain type (`BATTLE_ENVIRONMENT_...`) as based on where in the overworld the battle is occurring. Set at the start of the battle, used to set up the "intro slide" animation, used by some battle animations, and reset to `BATTLE_ENVIRONMENT_PLAIN` when playing an evolution cutscene. |
| `gBattleMonForms`[^tag-castform-form-change] | An array with one value per battler. Used to track Castform's current form during battles, contests, and similar scenes. Values are not expected to exceed `NUM_CASTFORM_FORMS`, and bit `1 << 7` is a sentinel flag used by some battle animations to indicate that Castform is behind a Substitute and so shouldn't play a form-change animation. Managed by `ClearBattleMonForms` and `docastformchangeanimation`; see also: `gBattleStruct->formToChangeInto`. |
| `gBattleOutcome` | Flags-mask indicating the outcome of the battle. Despite being a mask, many parts of the game treat it as a plain enum, doing equality comparisons instead of bitwise checks. |
| `gBattleWeather` | The current weather condition. Exposed to battle scripts, battle animation scripts, battle controllers, NPC AI, and even damage calculation internals. |
| `gBideDmg` | An array with one value per battler ID. Tracks damage received by a Pokemon while it's in the wait stages of using Bide. Reset to zero by `setbide`, increased by `datahpupdate`, and eventually applied as outgoing damage when processing the `CANCELLER_BIDE` attack canceler. |
| `gBideTarget` | An array with one value per battler ID. Tracks the last battler to inflict damage on a Pokemon while the latter is in the wait stages of using Bide. The `CANCELLER_BIDE` function uses this to know what Pokemon should be retaliated against when it's time for Bide to hit. |
| `gLastHitBy` | An array, with one value per battler ID. Indicates which other battler the subject was last hit by. This isn't updated if the battler faints. (Is there any particular reason for that, or does Game Freak just not bother?) |
| `gLastMoves` | An array, mapping battler IDs to the ID of the last move they obeyed the player's choice to use (i.e. `gChosenMove` if the Pokemon doesn't disobey). |
| `gLastPrintedMoves` | <p>An array, mapping battler IDs to the ID of the last move that **a)** they obeyed the player's choice to use (i.e. `gChosenMove`) and **b)** was printed to the screen (i.e. `HITMARKER_ATTACKSTRING_PRINTED`).</p><p>If a Pokemon uses Sketch, these are the moves they can potentially copy.</p> |
| `gLastResultingMoves` | An array, mapping battler IDs to the ID of the last move they actually used (i.e. `gCurrentMove` if the Pokemon doesn't disobey). |
| `gLockedMoves` | An array, mapping a battler ID to the ID of a move it's been locked into using. When it comes time for the battler to actually attack, this will overwrite `gChosenMove` and `gCurrentMove` if the battler has `STATUS2_MULTIPLETURNS` or `STATUS2_RECHARGE` (see `HandleAction_UseMove`). |
| `gMoveToLearn` | Used throughout all move-learning code, both during and outside of battles, to keep track of what move a Pokemon is being given an opportunity to learn. |
| `gPaydayMoney` |
| `gSentPokesToOpponent` | <p>An array of two bitmasks &mdash; one per enemy that can be simultaneously present on the field. The masks track which of the player's Pokemon have been fielded in battle against a given opponent. This is updated by `OpponentSwitchInResetSentPokesToOpponentValue`, and used by the `getexp` battle script command to know which of the player's Pokemon to award EXP to when an opponent is defeated.</p><p>Because this only tracks state for the opponent's two battle positions, one can infer that should the opponent switch a Pokemon out and back in, this state would be cleared for that Pokemon. That is: suppose the opponent sends out Pikachu, you send out Misdreavus, you switch out to Gastly, the opponent switches to Wobbuffet, and then the opponent switches back to Pikachu: your Misdreavus would no longer count as having been fielded against that Pikachu.</p> |
| `gSideStatuses` | An array of status flags -- one bitmask per side of the battlefield (player and enemy). |

### Ephemeral

| Name | Purpose |
| :- | :- |
| `gActiveBattler` | A battler ID &mdash; typically "the battler with which we are presently concerned." Typically used as a loop counter by several high-level latent functions within the battle engine that need to perform a task for all battlers. |
| `gBattlerAttacker` |
| `gBattlerFainted` | State variable for latent `HandleFaintedMonActions` function. Also exposed to battle scripts: it can be queried directly by commands like `jumpifbytenotequal`, and is checked by `BattleScript_LinkHandleFaintedMonMultiple`. |
| `gBattlerTarget` |
| `gEffectBattler` |
| `gChosenMove` | The move that `gBattlerAttacker` has initiated using. This can be the move they've chosen to use; *or* if they're locked into a move (e.g. Rage, Encore), it'll be that move; *or*, if they have no usable moves, it'll be Struggle. |
| `gCurrentMove` | By default, this is equal to `gChosenMove`. However, if using one move results in another move being used (e.g. using Mirror Move), this will equal that other used move. |

`gActiveBattler` is set when...

* ...a battle controller is sent any message. At the time they receive a message, battle controllers always assume that `gActiveBattler` is the battler that they're acting on behalf of, and they read message data out of `gBattleBufferA[gActiveBattler][n]`.
  * If one script command dispatches to a battle controller, and another script command expects to run afterward, then the latter must wait until all battle controllers are idle before proceeding, as `gActiveBattler` isn't safe to overwrite while messages outbound to a battle controller haven't yet been processed. As an example, `dofaintanimation` waits for controller idle, dispatches a message, and then continues onward without waiting for the message to be handled; and then `cleareffectsonfaint`, which is always placed after `dofaintanimation`, also waits for controller idle, and generally just behaves the same way.
    * In general, any battle script command that writes to `gActiveBattler` should wait until controller idle. Concerningly, not all of them do: `jumpifstatus3condition`, `healthbar_update`, and `playanimation`, among others, don't wait. Using `playanimation` as a case study, we see that it's generally either the first thing done in a script, or invoked after a `wait...` command such as `waitmessage` or `waitanimation`.
    * The `end` and `end2` commands set `gActiveBattler` to 0.
* ...native code intends to invoke a battle script that uses `printstring`, as that command dispatches a message to battle controllers with the active battler as the subject.
  * When Reflect, Light Screen, Mist, or Safeguard wear off for one side of the battlefield, the Pokemon that originally casted it is marked as the active battler. `BattleScript_SideStatusWoreOff` is queued for execution, and that uses `prinstring`.
  * Wish's end-turn effect `ENDTURN_WISH` may invoke `BattleScript_WishComesTrue`, which uses `printstring`.
  * `DoBattlerEndTurnEffects` preemptively sets the active battler and the attacker to whatever battler it's currently processing end-turn effects for. It may invoke multiple scripts that use `printstring`. There's no apparent reason besides `printstring` for it to touch the active battler variable, not least because it should only end up activating one effect for one Pokemon at a time.
  * `HandleWishPerishSongOnTurEnd` preemptively sets the active battler to whichever Pokemon it's processing, because it may invoke scripts.
  * Handlers for Shed Skin set the active battler when invoking scripts.
  * `ABILITYEFFECT_TRACE` sets the active battler to whoever is being Traced. (The Pokemon doing the Tracing is `gBattleScripting.battler` i.e. the *script*-active battler.)
* ...native code calls `PrepareStringBattle` (the underlying function for `printstring`). Notably, some native code sets `gActiveBattler` before calling this function, even though the function takes the desired battler as an argument, overwrites `gActiveBattler` with that battler, and doesn't restore it afterwards (since you have to wait for a battle controller to finish processing before it'd be safe to do so).
* `ItemBattleEffects` sets the active battler and the attacker to whoever is using the item.
* `clearstatusfromeffect` sets the active battler to an opcode argument. It's not immediately clear why.
* Specific cases
  * Non-latent functions repurposing this variable as a temporary for some reason, and clobbering it
    * In Double Battles, `HandleAction_UseMove` checks if the to-be-used move may potentially be redirected away from its intended target by Lightning Rod. If so, it loops over all enemy battlers besides the intended target, searching all battler IDs for an enemy Pokemon that has Lightning Rod and occupies the lowest index in `gBattlerByTurnOrder`; `gActiveBattler` is repurposed as the loop index. If a battler is found, `gActiveBattler` is then (without any apparent necessity) set to that battler; otherwise, it's left clobbered with an effectively meaningless value.
    * In link battles, `HandleAction_Run` searches all battlers for any that chose action `B_ACTION_RUN`, using `gActiveBattler` as the loop index. Based on which such battlers it finds, it sets bits `B_OUTCOME_LOST` or `B_OUTCOME_WON` in `gBattleOutcome`. 

### Latent

| Name | Used by | Description |
| :- | :-: | :- |
| `gBattlerStatusSummaryTaskId` | battle controllers | One value per battler ID. Used as latent state by the battle controllers, to store the task ID used to display the Pokemon Summary screen. |

### During a turn

| Name | Purpose |
| :- | :- |
| `gActionSelectionCursor` | For each battler, the position of the cursor used to select an action. This is used as a latent state variable by battle controllers; however, [Battle TV hooks][link-battle-tv-hooks] also use it to know what action the player chose. |
| `gActionsByTurnOrder` | Battlers' chosen actions, in turn order, such that `gActionsByTurnOrder[gBattlerByTurnOrder[n]]` is the action of battler ID <var>n</var>. |
| `gBattleMoveDamage` |
| `gBattlerByTurnOrder` | Battler IDs sorted by the order in which each battler will act during the current turn. |
| `gChosenActionByBattler[n]` | The action chosen by battler <var>n</var>, as tracked and handled by `HandleTurnActionSelectionState`. |
| `gCurrentActionFuncId` |
| `gCurrentMove` |
| `gCurrentTurnActionNumber` | Which turn action is currently being processed, such that `gBattlerByTurnOrder[gCurrentTurnActionNumber]` is the ID of the battler taking that action. |
| `gDynamicBasePower` |
| `gHitMarker` |
| `gMoveResultFlags` |
| `gMoveSelectionCursor` | For each battler, the position of the cursor used to select an action. This is used as a latent state variable by battle controllers; however, [Battle TV hooks][link-battle-tv-hooks] also use it to know what move the player chose. |

## `DisableStruct`
<a name="gDisableStructs"></a>

One instance exists per battler, stored in the `gDisableStructs` array. These values persist for as long as the battler is on the field, being cleared by `SwitchInClearSetData` and `FaintClearSetData`.

Field list (alphabetized):

| Name | Desciption |
| :- | :- |
| `battlerPreventingEscape` | Tracks which battler ID has applied `STATUS2_ESCAPE_PREVENTION` to the subject. If that battler faints or switches out (without using Baton Pass), then the status will be cleared from the subject. |
| `battlerWithSureHit` |
| `chargeTimer` |
| `chargeTimerStartValue` |
| `disableTimer` |
| `disableTimerStartValue` |
| `disabledMove` |
| `encoreTimer` |
| `encoreTimerStartValue` |
| `encoredMove` |
| `furyCutterCounter` |
| `isFirstTurn` | Set to 2 at the start of a battle (`BattleStartClearSetData`) and when a Pokemon is switched in (`SwitchInClearSetData`), and decremented at the end of each turn if non-zero (`TurnValuesCleanUp`). Used to activate Speed Boost on the first turn a Pokemon is in battle *after* the first turn of the *entire* battle. Also influences AI behavior (e.g. willingness to use Guard Spec and X-Items). |
| `mimickedMoves` | Bitmask indicating which of this battler's four moves were temporarily "learned" via Mimic. Checked indirectly via the `MOVE_IS_PERMANENT` macro, and maintained by the UI for reordering moves (i.e. `HandleMoveSwitching` within the player battle controller). |
| `perishSongTimer` |
| `perishSongTimerStartValue` |
| `protectUses` |
| `rechargeTimer` |
| `rolloutTimer` |
| `rolloutTimerStartValue` |
| `stockpileCounter` |
| `substituteHP` |
| `tauntTimer` |
| `tauntTimer2` |
| `truantCounter` |
| `truantSwitchInHack` |
| `transformedMonPersonality` |

## `ProtectStruct`
<a name="gProtectStructs"></a>

This struct describes battler state scoped to the duration of a single turn. One instance exists per battler, stored in the `gProtectStructs` array. All instances are cleared (reset to all-zero data) by `TurnValuesCleanUp` in `battle_main.c`.

(A lot of these fields are tracked solely for the sake of the `WasUnableToUseMove` function, which multiple `ENDTURN_...` effects call.)

Field list:

| Name | Description |
| :- | :- |
| `protected` |
| `endured` |
| `noValidMoves` | Updated by `AreAllMovesUnusable` in `battle_util.c`. Indicates that a Pokemon has no usable moves, and must use Struggle. |
| `helpingHand` | Indicates whether the subject has the Helping Hand buff. Set via the `trysethelpinghand` battle script command, which only allows the buff to activate if neither the attacker nor the target already have it. Checked by the `damagecalc`, `stockpiletobasedamage`, `trydobeatup`, and `trysetfutureattack` battle script commands, and by the internal `AI_CalcDmg` helper function for script commands. |
| `bounceMove` |
| `stealMove` | Indicates that the subject has used Snatch and is waiting to steal a move. Originally set by `trysetsnatch`, its effect is applied in `attackcanceler`. |
| `flag0Unknown` |
| `prlzImmobility` | Boolean, set by `AtkCanceller_UnableToUseMove` the first time a Pokemon is unable to act due to paralysis. |
| `confusionSelfDmg` | Boolean, set when a Pokemon hurts itself due to confusion. Prevents effects like Rough Skin from affecting the damage dealt. |
| `targetNotAffected` |
| `chargingTurn` |
| `fleeType` |
| `usedImprisonedMove` |
| `loveImmobility` |
| `usedDisabledMove` |
| `usedTauntedMove` |
| `flag2Unknown` |
| `flinchImmobility` | Boolean, set when a Pokemon fails to attack due to flinching. |
| `notFirstStrike` |
| `palaceUnableToUseMove` |
| `physicalDmg` | Tracks the damage received from the last physical attack that hit the subject. Set by `datahpupdate`. Later used by `counterdamagecalculator`, `doubledamagedealtifdamaged`, and `jumpifnodamage`. An apparently unused copy of this field exists in `SpecialStatus`. |
| `specialDmg` | Tracks the damage received from the last special attack that hit the subject. Set by `datahpupdate`. Later used by `mirrorcoatdamagecalculator`, `doubledamagedealtifdamaged`, `jumpifnodamage`, and `moveend` (for latent effect `MOVEEND_DEFROST`). An apparently unused copy of this field exists in `SpecialStatus`. |
| `physicalBattlerId` | Tracks the last Pokemon to hit the subject with a physical attack, for the sake of Counter. Set by `datahpupdate`. Later used by `counterdamagecalculator` and `doubledamagedealtifdamaged`. An apparently unused copy of this field exists in `SpecialStatus`. |
| `specialBattlerId` | Tracks the last Pokemon to hit the subject with a special attack, for the sake of Mirror Coat. Set by `datahpupdate`. Later used by `mirrorcoatdamagecalculator` and `doubledamagedealtifdamaged`. An apparently unused copy of this field exists in `SpecialStatus`. |

## `SpecialStatus`
<a name="gSpecialStatuses"></a>

One instance exists per battler, stored in the `gSpecialStatuses` array. All instances are cleared by the latent `SpecialStatusesClear` function, which is invoked at the start of the battle, before the first turn, and after any turn action is finished (see `HandleAction_ActionFinished`).

Fields:

| Name | Description |
| :- | :- |
| `statLowered` | <p>Used to suppress redundant messages, when a Pokemon gets hit by a move that reduces multiple stats while the Pokemon is protected from all stat losses.</p><p>Read and written by the internal `ChangeStatBuffs` helper function for battle script commands, which is ultimately called by `statbuffchange`. Used so that if a Pokemon is protected from stat loss (i.e. by Mist, Clear Body, or White Smoke), and multiple move effects in the current action try to lower its stats, you're only told about the stat loss protection once. Some moves (e.g. Curse) are special-cased to bypass this behavior. Whether this behavior activates depends on the arguments passed to `statbuffchange`, as well as the stat change prepared for it by a previous `setstatchanger` command.</p> |
| `lightningRodRedirected` |
| `restoredBattlerSprite` |
| `intimidatedMon` |
| `traced` | Tracks whether the subject's Trace ability has activated during this turn. Used to confer `STATUS3_TRACE` to the subject only once, so that if the subject Traces Trace, it won't then Trace something else. Trace only activates when switching in, so the fact that these flags get cleared after each action won't allow Trace to re-apply. |
| `ppNotAffectedByPressure` | <p>Indicates that the Pokemon shouldn't be affected by the `ppreduce` battle script command. Used when Mirror Move fails (`trymirrormove`), when Counter fails (`counterdamagecalculator`), and when Mirror Coat fails (`mirrorcoatdamagecalculator`), when Spikes fails (`trysetspikes`). Also applied indiscriminately to Magic Coat (`trysetmagiccoat`) and Snatch (`trysetsnatch`).</p><p>This is similar in function to `HITMARKER_NO_PPDEDUCT`, except that the hitmarker only blocks the effect of the very next `ppreduce` call, whereas this field blocks the effects of all `ppreduce` calls until the turn action completes. The hitmarker is used for the second turn of a two-turn move, for moves utilized via Sleep Talk (but not for Sleep Talk itself), for the "bounce" effect of Magic Coat, and for moves utilized via Snatch (but not for Snatch itself).</p> |
| `faintedHasReplacement` |
| `focusBanded` |
| `shellBellDmg` |
| `physicalDmg` | Tracks the damage received from the last physical attack that hit the subject. Set by `datahpupdate`. This field is only used by the `TARGET_TURN_DAMAGED` macro (itself used by `AbilityBattleEffects` latent case `ABILITYEFFECT_ON_DAMAGE`), but a counterpart exists in `ProtectStruct` and is used for more purposes. |
| `specialDmg` | Tracks the damage received from the last special attack that hit the subject. Set by `datahpupdate`. This field is only used by the `TARGET_TURN_DAMAGED` macro, but a counterpart exists in `ProtectStruct` and is used for more purposes. |
| `physicalBattlerId` | Tracks the last Pokemon to hit the subject with a physical attack. Set by `datahpupdate`. This field seems to be unused here, but also exists in `ProtectStruct` and is used there. |
| `specialBattlerId` | Tracks the last Pokemon to hit the subject with a special attack. Set by `datahpupdate`. This field seems to be unused here, but also exists in `ProtectStruct` and is used there. |

## `SideTimer`
<a name="gSideTimers"></a>

Each side of the battlefield (player and enemy) has its own instance, as `gSideTimers[n]`. Broadly speaking, these data structures manage the durations and intensities of "side statuses" (i.e. `gSideStatuses[n]`).

Fields:

| Name | Description |
| :- | :- |
| `reflectTimer` | Number of turns for which `SIDE_STATUS_REFLECT` will continue to be active. |
| `reflectBattlerId` | ID of the battler that cast `SIDE_STATUS_REFLECT`. Used for the messages shown when the status wears off. |
| `lightscreenTimer` |
| `lightscreenBattlerId` |
| `mistTimer` |
| `mistBattlerId` |
| `safeguardTimer` |
| `safeguardBattlerId` |
| `followmeTimer` | If non-zero, this side of the field has Follow Me active, and attacks inbound from the other side of the field will be redirected to the Follow Me target. |
| `followmeTarget` | Target (battler ID) to which attacks from the other side of the field will be redirected, while the Follow Me timer is active. |
| `spikesAmount` | Indicates the amount of spikes scattered on this side of the battlefield by Spikes. Increased by `trysetspikes` up to a maximum of 3. Influences the damage dealt by Spikes. This value is usually only checked if `gSideStatuses[n]` has `SIDE_STATUS_SPIKES`. |

## `WishFutureKnock`
<a name="gWishFutureKnock"></a>

A singleton accessible via `gWishFutureKnock`. This manages the counters and state associated with Future Sight/Doom Desire, Wish, Knock Off, and moves that trigger weather effects. These values are fully persistent: even if the battler faints or is switched out, these values are kept in place and remain in effect.

Fields:

| Name | Description |
| :- | :- |
| `futureSightCounter` | Given a target `x`, `futureSightCounter[x]` is the number of turns left until the attack hits. |
| `futureSightAttacker` | Given a target `x`, `futureSightAttacker[x]` will be the ID of the attacker. |
| `futureSightDmg` | Given a target `x`, `futureSightDmg[x]` is the damage that `x` will take when Future Sight hits.
| `futureSightMove` | Given a target `x`, `futureSightMove[x]` is the ID of the move that will hit. |
| `wishCounter` | Given a caster `x`, `wishCounter[x]` is the number of turns left until the move's effect activates. |
| `wishMonId` | Given a caster `x`, `wishMonId[x]` is the position of `x` within its containing party, such that Wish can still affect it after it's switched out. |
| `weatherDuration` | The number of turns for which the current weather will last. |
| `knockedOffMons` | Two bitmasks, one per battlefield side. Each mask has one bit per party member, indicating which party members have lost their items to Knock Off. |


## `BattleStruct`
<a name="gBattleStruct"></a>

A singleton accessible via the pointer `gBattleStruct`.

Fields:

| Name | Purpose | Description |
| :- | :- | :- |
| `turnEffectsTracker` | **Latent:** `DoBattlerEndTurnEffects` | Latent state for `DoBattlerEndTurnEffects`: which of the possible end-of-turn battler effects are we processing? |
| `turnEffectsBattlerId` | **Latent:** `DoBattlerEndTurnEffects` | Latent state for `DoBattlerEndTurnEffects`: for which battler are we processing end-of-turn battler effects? |
| `unused_0` |
| `turnCountersTracker`[^tag-DoFieldEndTurnEffects-latent-state] | **Latent:** `DoFieldEndTurnEffects` | Latent state for `DoFieldEndTurnEffects`: which of the possible end-of-turn field effects are we processing?
| `wrappedMove` | **Status:** Bound | Array; one value per battler. Indicates the move ID that was used to inflict `STATUS2_WRAPPED` on the subject. Used to display the appropriate move name and text strings when the status inflicts damage or is ended. |
| `moveTarget` | **System** | Array; one value per battler. Maps attacker IDs to their target IDs. Set by `HandleTurnActionSelectionState` in response to battle controllers' actions. Overwritten by `HandleAction_UseMove` in special cases (e.g. Struggle; attacker forced to use a move other than what they chose; etc.); the function then uses this variable (when appropriate) to set `gBattlerTarget`. |
| `expGetterMonId`[^tag-getexp-latent-state] | **Latent:** `getexp` | Latent state for the `getexp` command, indicating which party slot we're considering awarding EXP to. |
| `unused_1` |
| `wildVictorySong` | **System** | Indicates whether the background music for defeating a Wild Pokemon has started playing yet. That music is triggered by the `getexp` script command, so this ensures that the music doesn't restart when multiple Pokemon gain EXP from defeating a single Wild Pokemon. |
| `dynamicMoveType` | **Feature:** moves that change type | When inactive, zero. When active, a `TYPE_...` constant bitwise-OR'd with `F_DYNAMIC_TYPE_SET` and optionally bitwise-OR'd with `F_DYNAMIC_TYPE_IGNORE_PHYSICALITY`. Used for moves that have a variable type, such as Hidden Power and Weather Ball. Set by `hiddenpowercalc` and `setweatherballtype`. Read by the `GET_MOVE_TYPE` macro, and also read by various callees for functions that calculate damage. Reset by `CheckFocusPunch_ClearVarsBeforeTurnStarts`. |
| `wrappedBy` | **Status:** Bound | Array; one value per battler. Given a battler `n`, the value of `gBattleStructs->wrappedBy[n]` is the ID of the battler that inflicted `STATUS2_WRAPPED` on battler `n`. (If battler `n` does not have `STATUS2_WRAPPED`, then the field is meaningless for them.) Set by the internal `SetMoveEffect` helper function for script commands, when processing `MOVE_EFFECT_WRAP`; and read by `rapidspinfree` for text display when a Pokemon frees itself from `STATUS2_WRAPPED`. Cleared when the wrapping Pokemon switches out (`SwitchInClearSetData`) or faints (`FaintClearSetData`). |
| `assistPossibleMoves` | **Move:** Assist | An array of move IDs referenced exclusively within `assistattackselect`; this could've been a local variable. The `assistattackselect` command gathers all move IDs known to the attacker's (non-egg) allies, filters out any moves that are disallowed for Sleep Talk and Assist, and stores the rest in this array. Then, it picks one at random. |
| `focusPunchBattlerId` | **Latent:** `CheckFocusPunch_ClearVarsBeforeTurnStarts` | Latent state for the `CheckFocusPunch_ClearVarsBeforeTurnStarts` function. Indicates which battler we're currently processing Focus Punch turn-end effects on. |
| `battlerPreventingSwitchout`[^tag-party-menu-latent-state] | party menu | Latent state for the party menu, when opened by the player battle controller. The battle controller will be provided with: the slot number of the party memeber that the player would be switching out; the battler ID preventing a switch-out, if any; and the ability preventing a switch-out, if any. These are received via a battler controller message, so the battle controller stashes them here so that the party menu can use them for logic (i.e. preventing you from switching a Pokemon out with itself) and error messaging (for when a switch-out is blocked). |
| `moneyMultiplier` | **System** | Initialized to 1 at the start of a battle. Held items with the `HOLD_EFFECT_DOUBLE_PRIZE` effect (e.g. Amulet Coin) force it to 2 as a switch-in effect. The variable is eventually used by the `getmoneyreward` and `givepaydaymoney` battle script commands. |
| `savedTurnActionNumber` | **Latent:** `RunTurnActionsFunctions` | Latent state for the `RunTurnActionsFunctions` function. The function sets this to `gCurrentTurnActionNumber`, runs processing for turn actions, and then compares the two variables to see if they differ. This allows the function to know when to clear certain hitmarker bits between battlers. |
| `switchInAbilitiesCounter`[^tag-TryDoEventsBeforeFirstTurn-latent-state] | **Latent:** `TryDoEventsBeforeFirstTurn` | Latent state for `TryDoEventsBeforeFirstTurn`, indicating which battler we're processing switch-in ability effects for. |
| `faintedActionsState`[^tag-HandleFaintedMonActions-latent-state] | **Latent:** `HandleFaintedMonActions` | Latent state for `HandleFaintedMonActions`, indicating what stage of execution the function is in. |
| `faintedActionsBattlerId`[^tag-HandleFaintedMonActions-latent-state] | **Latent:** `HandleFaintedMonActions` | Latent state for `HandleFaintedMonActions`, indicating what battler the function is processing. |
| `expValue`[^tag-getexp-latent-state] | **Latent:** `getexp` | Latent state for the `getexp` command, storing the total EXP earned. |
| `scriptPartyIdx` | **Feature:** switch-out messages | <p>Unused. The `B_TXT_26` placeholder code for battle messages uses this to pull a Pokemon's nickname. This value is set when switching out a Pokemon (see `hpOnSwitchout`), but the code for switch-out messages doesn't use it; they just buffer the Pokemon's nickname.</p> |
| `sentInPokes`[^tag-getexp-latent-state] | **Latent:** `getexp` | Latent state for the `getexp` command: a bitmask indicating which of the player's Pokemon have been fielded in battle against the defeated opponent. Computed from `gSentPokesToOpponent`. |
| `selectionScriptFinished` | **System:** Actions | Array; one entry per battler ID. If a battler is actively running an action [selection script][link-action-selection-script], this tracks whether the script has finished executing. The latent `HandleTurnActionSelectionState` function sets this variable to `FALSE` and waits for `endselectionscript` to set it to `TRUE`. |
| `battlerPartyIndexes` | **System** | <p>This field's name is easily confused with `gBattlerPartyIndexes`, but the two variables serve fundamentally different purposes.</p><p>When the battle controller for battler <var>n</var> decides to switch that Pokemon out, <code>gBattleStruct->battlerPartyIndexes[<var>n</var>]</code> holds the party slot that the to-be-withdrawn battler occupied prior to the switch. This value is set by `HandleTurnActionSelectionState` when handling `B_ACTION_SWITCH`, and then later used by `HandleAction_Switch` to buffer the name of the to-be-withdrawn Pokemon in advance of running the `BattleScript_ActionSwitch` script.</p><p>I assume this makes it easier, in some way, to implement the display of switch-out messages during Double Battles, should a trainer switch out both of their Pokemon in the same turn.</p> |
| `monToSwitchIntoId` | **System** | When a battle controller asks to switch out battler ID <var>n</var>, `monToSwitchIntoId[n]` will hold the party slot of the Pokemon to switch in as a replacement. |
| `battlerPartyOrders` | **System:** Multi Battles | For each battler, an array of three party indices. Related to Multi Battles. Exact functionality unclear. |
| `runTries` | **System** | Number of times the player has attempted to flee from battle. The more times you try to run, the more likely the next try is to succeed. Only incremented if the player doesn't flee using a guaranteed-success ability or held item. |
| `caughtMonNick` || Latent state for the `trygivecaughtmonnick` battle script command, *and* checked after a battle by the TV code (i.e. `TryPutPokemonTodayOnAir` in `tv.c`). |
| `unused_2` |
| `safariGoNearCounter` | **Feature:** Safari Zone |
| `safariPkblThrowCounter` | **Feature:** Safari Zone |
| `safariEscapeFactor` | **Feature:** Safari Zone |
| `safariCatchFactor` | **Feature:** Safari Zone |
| `linkBattleVsSpriteId_V` || During Link Battles, this is the ID of a "V" sprite, forming a "VS" graphic. |
| `linkBattleVsSpriteId_S` || During Link Battles, this is the ID of a "S" sprite, forming a "VS" graphic. |
| `formToChangeInto`[^tag-castform-form-change] | Castform form change | Latent state for Castform's form change: used to pass the desired form from the hardcoded handler for `ABILITY_FORECAST`, to and through the battle script command `docastformchangeanimation`, through the battle controller, to the battle animation `B_ANIM_CASTFORM_CHANGE`. |
| `chosenMovePositions` | **System:** Actions | When a battler has chosen to use the move, this is a value in the range [0, 4) indicating which move (as a position within the battler's moveset) they chose to use. |
| `stateIdAfterSelScript` | **System:** Actions | An array with one value per battler. When a battler is running an action [selection script][link-action-selection-script], this holds the action selection state they should advance to after the script finishes executing. |
| `unused_3` |
| `prevSelectedPartySlot`[^tag-party-menu-latent-state] | party menu | See `battlerPreventingSwitchout`. |
| `unused_4` |
| `stringMoveType` | battle messages | Set by `BufferStringBattle` to the value of `gBattleMsgDataPtr->moveType`, and then copied immediately afterward into the appropriate buffers for string substitutions, based on the message to display. Also checked by `BattleStringExpandPlaceholders` (which is invoked at the end of `BufferStringBattle`). In all cases, it's used to display out-of-bounds move IDs with failsafe strings like "a NORMAL move." |
| `expGetterBattlerId`[^tag-getexp-latent-state] | **Latent:** `getexp` | Latent state for the `getexp` command, indicating for which battler we're currently processing potential EXP earnings. |
| `unused_5` |
| `absentBattlerFlags` | **System** | <p>Unclear. Related to the `gAbsentBattlerFlags` global.</p><p>The `gAbsentBattlerFlags` global is synchronized over the network, and any time a flag is modified directly, it's in `gAbsentBattlerFlags`. At the start and end of each turn, `gBattleStruct->absentBattlerFlags` is overwritten with the current value of `gAbsentBattlerFlags`. The `HandleTurnActionSelectionState` function is the only place that reads this value instead of `gAbsentBattlerFlags`, but given how critical that function is to the core battle engine, that may be for a reason.</p> |
| `palaceFlags` |
| `field_93` |
| `wallyBattleState` | **Latent:** `WallyHandleActions` | Latent state for Wally's battle controller. Wally has a fixed list of actions that he'll take; this indicates which action he's on. |
| `wallyMovesState` | **Latent:** `WallyHandleActions` | Latent state for Wally's battle controller. This variable indicates what "step" he's on when choosing which of Zigzagoon's moves to attack with. |
| `wallyWaitFrames` | **Latent:** `WallyHandleActions` | Latent state for Wally's battle controller: a countdown timer to handle the delays between his actions. |
| `wallyMoveFrames` | **Latent:** `WallyHandleActions` | Latent state for Wally's battle controller: a countdown timer to handle the delays between his "steps" when choosing which of Zigzagoon's moves to attack with. |
| `lastTakenMove` | **Move:** Mirror Move | PRET lists this as a 16-byte buffer, but it's better understood as `u16 field[MAX_BATTLERS_COUNT * 2]`, where `field[n]` indicates the ID of the last move that battler ID <var>n</var> was hit with. It's not clear why the array is twice as large as it needs to be. See also: `lastTakenMoveFrom`. |
| `hpOnSwitchout` | **Feature:** Switch-out messages | Array; one value per battlefield side, so this is only suitable for Single Battles and Multi Battles. Indicates the HP that the Pokemon on that side of the field had at the time it was switched out of battle. Updated by `BattleIntroDrawTrainersOrMonsSprites`, `CopyPlayerPartyMonToBattleData` (`pokemon.c`), and `switchineffects`. Read by `hpthresholds2`, which (outside of Double Battles) adjusts the value of `gBattleStruct->hpScale`. Influences (via `hpScale`) the UI message shown when the player switches out their Pokemon. |
| `savedBattleTypeFlags` | **Latent:** `CB2_PreInitMultiBattle`/`CB2_PreInitIngamePlayerPartnerBattle` | Unclear. These functions use this to back up `gBattleTypeFlags` at the start of its processing, and restore it at the end; but it's not clear how `gBattleTypeFlags` would ever change in the interim. |
| `abilityPreventingSwitchout`[^tag-party-menu-latent-state] | party menu | See `battlerPreventingSwitchout`. |
| `hpScale` | **Feature:** Switch-out messages | Updated by `hpthresholds` and `hpthresholds2` based on the value of `gBattleStruct->hpOnSwitchout`. Controls the UI message shown when the player switches out their Pokemon. |
| `synchronizeMoveEffect` | **Ability:** Synchronize | A `MOVE_EFFECT_...` value. Set by `AbilityBattleEffects` when processing `ABILITYEFFECT_SYNCHRONIZE`/`ABILITYEFFECT_ATK_SYNCHRONIZE`, before triggering execution of the `BattleScript_SynchronizeActivates` script. That script runs the `seteffectprimary` command, which calls the internal `SetMoveEffect` helper function, which reads and potentially modifies this field. Sometimes, `gBattleCommunication[MOVE_EFFECT_BYTE]` is modified alongside this field; sometimes, this field is reset to `gBattleCommunication[MOVE_EFFECT_BYTE]`; maybe I need to come at this with more context, but right now it's not clear what's going on. |
| `anyMonHasTransformed` | battle TV | Set by `BattleTv_SetDataBasedOnString` when printing a message about a Pokemon transforming. Prevents `TryPutLinkBattleTvShowOnAir` from putting the battle on TV. |
| `(*savedCallback)(void)` | **Latent:** `CB2_PreInitMultiBattle`/`CB2_PreInitIngamePlayerPartnerBattle` | Unclear. These functions use this to back up `gMain.savedCallback` at the start of its processing, and restore it at the end. |
| `usedHeldItems` | **Move:** Recycle | <p>Array; one item ID per battler ID. When a Pokemon's held item is used or otherwise destroyed (see `removeitem`), the item ID is stored here, so that it can be recovered via the `tryrecycleitem` script command (i.e. the Recycle move).</p><p>This system is imperfect: lost/used items should ideally be tied to battlers (i.e. side and party position), not battlefield positions; that they're tied to the latter is part of what enables Mail corruption exploits.</p> |
| `chosenItem` | NPC AI | Latent state for NPC AI and the NPC opponent battle controller. The NPC AI writes an item here, and then the NPC opponent battle controller reads the value from here and sends it in an outbound message. This is an item ID, but it's stored as a `u8`, so problems are likely to occur if somehow an NPC AI uses an item ID above 255.[^npc-ai-item-usage] |
| `AI_itemType` | NPC AI | One value per enemy battler; each is an `AI_ITEM_...` constant. Stored by `ShouldUseItem`, an internal function in the NPC AI code; read by `HandleAction_UseItem` if the AI commits to using the item.[^npc-ai-item-usage] |
| `AI_itemFlags` | NPC AI | One flags-mask value per enemy battler. Stored by `ShouldUseItem`, an internal function in the NPC AI code; read by `HandleAction_UseItem` if the AI commits to using the item. With respect to NPC AI, all status-curing items and all X-items are the same `AI_ITEM`, and are distinguished from one another using these flags.[^npc-ai-item-usage] |
| `choicedMove` || Array: one move ID per battler. If set to anything other than `MOVE_NONE`, indicates that the battler has been locked into the given move by Choice Band or a similar item. |
| `changedItems` || <p>When a Pokemon's held item is changed by Thief, Trick, or any similar effect, this stores the incoming item. The value gets moved into `gBattleMons[i].item` as a move-end effect (`MOVEEND_CHANGED_ITEMS`). This is used to ensure the correct ordering of operations with respect to things like Choice Band.</p><p>If your Pokemon steals a Choice Band using Thief, then the band gets moved into this field, and move-end effects run. First, the move-end effect for Choice Band (`MOVEEND_CHOICE_MOVE`) is processed: because the Choice Band is stored here, and *not* on your Pokemon, your Pokemon isn't erroneously affected by Choice Band and locked into using Thief. Then, immediately after the Choice Band move-end effect is processed, `MOVEEND_CHANGED_ITEMS` will move the item out of this field (leaving this field zeroed) and onto your Pokemon.</p> |
| `intimidateBattler` | **Ability:** Intimidate | Stores the ID of a battler whose Intimidate ability has activated, so that battle scripts can retrieve it via `trygetintimidatetarget`. Generally set when queueing Intimidate battle scripts to run. |
| `switchInItemsCounter`[^tag-TryDoEventsBeforeFirstTurn-latent-state] | **Latent:** `TryDoEventsBeforeFirstTurn` | Latent state for `TryDoEventsBeforeFirstTurn`, indicating which battler we're processing held item switch-in effects for. |
| `arenaTurnCounter` |
| `turnSideTracker`[^tag-DoFieldEndTurnEffects-latent-state] | **Latent:** `DoFieldEndTurnEffects` | Latent state for `DoFieldEndTurnEffects`, indicating which side of the battlefield we're processing effects for.
| `unused_6` |
| `givenExpMons` | **System:** EXP | A bitmask, with one bit per Pokemon in the enemy's party, indicating which such Pokemon have granted EXP to the player's party; ensures that if the enemy revives their Pokemon, the player can't earn extra EXP from fainting them more than once. Used by `HandleFaintedMonActions` to track whether it should trigger execution of `BattleScript_GiveExp`. Set by the `giveexp` script command to indicate that it has finished awarding EXP from the defeat of a given battler. |
| `lastTakenMoveFrom` | **Move:** Mirror Move | <p>PRET lists this as a byte buffer of size `MAX_BATTLERS_COUNT * MAX_BATTLERS_COUNT * 2`, but it's better understood as `u16 field[MAX_BATTLERS_COUNT][MAX_BATTLERS_COUNT]`, where `field[x][y]` indicates the ID of the last move that battler ID <var>y</var> hit battler ID <var>x</var> with. See also: `lastTakenMove`.</p><p>The `trymirrormove` script command reads from this and `lastTakenMove`.</p> |
| `castformPalette` | | <p>One color palette for each of Castform's forms. Any time Castform's graphics are loaded, all of its forms' color palettes are loaded together and stored here. This isn't a cache.</p><p>Functions exist for loading the front and back sprites of a Pokemon species, along with the appropriate palette. These functions load and decompress the palette into `gDecompressionBuffer`, and then pass the first palette's worth of data from there into VRAM. When these functions are directed to load data for `SPECIES_CASTFORM`, they do that (thereby loading Castform's normal palette into VRAM), but then decompress the palette data second time into `gBattleStruct->castformPalette`, before loading the palette for a specific Castform form from that array into VRAM. (The makefile for graphics data (`graphics_file_rules.mk`) contains special-case rules for Castform, so that its generated `gbapal` file includes the data for all of its forms combined together.) This is... redundant. Once the data's in `gDecompressionBuffer`, it should be trivial to just copy the appropriate form palette from there; this `castformPalette` field is wholly unnecessary.</p> |
| `multiBuffer.linkBattlerHeader` |
| `multiBuffer.battleVideo` |
| `wishPerishSongState`[^tag-HandleWishPerishSongOnTurnEnd-latent-state] | **Latent:** `HandleWishPerishSongOnTurnEnd` | Latent state for `HandleWishPerishSongOnTurnEnd`, indicating which stage of execution the function is in. |
| `wishPerishSongBattlerId`[^tag-HandleWishPerishSongOnTurnEnd-latent-state] | **Latent:** `HandleWishPerishSongOnTurnEnd` | Latent state for `HandleWishPerishSongOnTurnEnd`, indicating which battler is being processed. |
| `overworldWeatherDone` |
| `atkCancellerTracker` | **Latent:** `AtkCanceller_UnableToUseMove` | Latent state for `AtkCanceller_UnableToUseMove`, indicating which potential attack canceler we're currently processing. |
| `tvMovePoints` |
| `tv` |
| `unused_7` |
| `AI_monToSwitchIntoId` | **Latent:** NPC AI | <p>Latent state for the NPC opponent battle controller. If the NPC AI decides that it wants to switch due to Perish Song or Wonder Guard, it'll choose what party slot to switch in, store that here, and then select `B_ACTION_SWITCH`. Then, when it gets asked who it wants to switch in, it'll pull this variable's value, see that it isn't equal to `PARTY_SIZE` (the sentinel value for when the AI hasn't already decided who to switch in), and send that as a response.</p><p>The battle engine is courteous enough to reset this field in `BattleStartClearSetData`, but otherwise doesn't ever touch this field.</p> |
| `arenaMindPoints` | **Feature:** Battle Arena | <p>One value per battler: their current Mind points.</p><p>Despite having "one value per battler," this is actually only an array of two values, not four: the Battle Arena code assumes that all battles are single battles, and uses battler IDs to index blindly into this array without bounds-checking. If you were to implement Double Arena Battles without checking this (and probably a few other things), you'd run into some issues.</p> |
| `arenaSkillPoints` | **Feature:** Battle Arena |
| `arenaStartHp` | **Feature:** Battle Arena |
| `arenaLostPlayerMons` | **Feature:** Battle Arena |
| `arenaLostOpponentMons` | **Feature:** Battle Arena |
| `alreadyStatusedMoveAttempt` | **Feature:** Battle Arena | Bitmask; one bit per attacker. Set by the `setalreadystatusedmoveattempt` script command, which battle scripts invoke when a move that only inflicts a status (e.g. Thunder Wave) fails because the target already has that status. Seemingly only checked (and reset) by `BattleArena_AddSkillPoints`, which deducts 2 skill points from the attacker in this scenario. |

## `BattleScripting`

A small singleton accessible via `gBattleScripting`, which is often read and written directly by battle script commands.

A handful of fields on this struct are never actually used by battle scripts. This struct doesn't appear to ever be sent over a multiplayer link, so in theory, those fields could be moved to a more sensible place.

Multiple script commands have wholly separate "state" fields stored here. This is because battle scripts are a stack: scripts can invoke other scripts (including indirectly, e.g. if a script runs a script command that invokes a script, or if a script runs a script command that calls a native function that invokes a script, and so on). The script commands that need latent state are not reentrant, so the only "cost" is having a bunch of globals scattered every which way.

Fields:

| Offset | Name | Description |
| -: | :- | :- |
| 0x00 | `painSplitHp` | <p>The amount of HP that the target of Pain Split must lose.</p><p>Set by `painsplitdmgcalc`, and then read by the `BattleScript_EffectPainSplit` script, to copy it into `gBattleMoveDamage`. The `painsplitdmgcalc` command can't write this value to `gBattleMoveDamage` directly, because that global is needed to track the damage that the attacker inflicts upon themselves.</p><p>In essence, this value only exists because two Pokemon on the field are taking damage at the same time, with the same calculation determining the damage taken by each, and with that calculation dependent on any of the affected Pokemon's HP (i.e. the calculation isn't idempotent and can't simply be re-run). As far as I'm aware, it's a totally unique edge-case, so it's not worth trying to bake that functionality into the script engine just to remove the need for this one field.</p> |
| 0x04 | `bideDmg` | <p>Latent state for Bide.</p><p>The amount of damage to be inflicted by Bide.</p><p>When a Pokemon uses Bide and is building up energy, most damage inflicted upon them is stored in `gBideDmg[gActiveBattler]`, and the identity of their last attacker is stored in `gBideTarget[gActiveBattler]`. Bide's "flow," meanwhile, is implemented almost entirely within the `CANCELLER_BIDE` attack canceller, which handles either the "storing energy" message or the eventual attack. The attack involves the canceller pulling the to-be-inflicted damage into `gBattleScripting.bideDmg` and then queueing the `BattleScript_BideAttack` script to run. That script will eventually copy this value into `gBattleMoveDamage` and kick off the code for inflicting damage.</p> |
| 0x08 | `multihitString[6]` |
| 0x0E | `dmgMultiplier` | <p>Scales damage calculations. Modified by move scripts for bespoke effects, such as Twister dealing double damage to Pokemon that are currently in the air (i.e. using Fly).</p><p>Reset to 1 in `HandleAction_UseMove`, prior to move scripts running. NPC AI will reset this to 1 as well, presumably so it can reuse move code when computing a move's probable damage.</p> |
| 0x0F | `twoTurnsMoveStringId` |
| 0x10 | `animArg1` |
| 0x11 | `animArg2` |
| 0x12 | `tripleKickPower` | Scratch variable used within `BattleScript_EffectTripleKick`: initialized to 0, and increased by 10 with each hit. |
| 0x14 | `moveendState` | <p>This is latent state for the `moveend` script command, indicating which `MOVEEND_...` case is currently being processed. However, the `moveend` command was designed to allow three moves of execution, with the command's first argument being its mode: "Run all cases from <var>x</var> onward" (0); "Run all cases from <var>x</var> onward, stopping at the first case that doesn't match" (1); and "Run all cases from <var>x</var> through <var>y</var>" (2). Functionally, this value is <var>x</var>, and <var>y</var> is the second argument to the command.</p><p>Reset to 0 at the start of the battle and the end of each turn.</p> |
| 0x15 | `battlerWithAbility` | Set by the `jumpifability` script command, which checks if any battler on a given side of the field has a given ability. This is used in one (1) place: printing a message if a Pokemon's Insomnia negates the effect of Yawn. |
| 0x16 | `multihitMoveEffect` |
| 0x17 | `battler` | The "script active battler" &mdash; that is, the battler ID that `BS_SCRIPTING` refers to, and whose name will be printed by the placeholder code  `B_TXT_SCR_ACTIVE_NAME_WITH_PREFIX`. |
| 0x18 | `animTurn` |
| 0x19 | `animTargetsHit` |
| 0x1A | `statChanger` |
| 0x1B | `statAnimPlayed` | <p>Scripts should set this to `FALSE` before calling `playstatchangeanimation`. The script command will set it to `TRUE` if an animation is successfully played, and decline to play an animation if it's already `TRUE`.</p><p>The general pattern for using the script command is as follows (pseudocode, not actual script code):</p><pre>// presume we want to lower Attack and Defense<br/>sSTAT_ANIM_PLAYED = FALSE;<br/>if (target.attack > MIN_STAT_STAGE) {<br/>   playstatchangeanimation(target, attack \| defense, STAT_CHANGE_NEGATIVE \| STAT_CHANGE_MULTIPLE_STATS);<br/>   //<br/>   // If Defense can't go lower, then the above command fails and the below <br/>   // command runs. If Defense can go lower, then the above command runs and <br/>   // the below command fails.<br/>   //<br/>   playstatchangeanimation(target, attack, STAT_CHANGE_NEGATIVE);<br/>} else {<br/>   playstatchangeanimation(target, defense, STAT_CHANGE_NEGATIVE);<br/>}</pre> |
| 0x1C | `getexpState`[^tag-getexp-latent-state] | Latent state for the `getexp` command, indicating what step of execution the command is on. Scripts must set this to 0 before running the `getexp` command. |
| 0x1D | `battleStyle` | Set at the start of a battle by `BattleStartClearSetData`, to match the player's Battle Style option (Shift/Set). Checked by `BattleScript_FaintedMonTryChoose`, which uses it to implement the setting. |
| 0x1E | `drawlvlupboxState` |
| 0x1F | `learnMoveState` | Latent state for the `yesnoboxlearnmove` and `yesnoboxstoplearningmove` script commands, indicating which step of execution it's on. This must be set to 0 before running either command. |
| 0x20 | `pursuitDoublesAttacker` |
| 0x21 | `reshowMainState` | Latent state for `CB2_ReshowBattleScreenAfterMenu`: indicates what step of the process we're on. This isn't accessed by any battle scripts, but it's stored here nonetheless. |
| 0x22 | `reshowHelperState` | Latent state for `CB2_ReshowBattleScreenAfterMenu` and `BattleLoadAllHealthBoxesGfx`: indicates what step of the latter, inner, process we're on. This isn't accessed by any battle scripts, but it's stored here nonetheless. |
| 0x23 | `levelUpHP` | Set by `CalculateMonStats` and later read by `PokemonUseItemEffects` (both in `pokemon.c`), if the latter is processing the use of a Rare Candy. |
| 0x24 | `windowsType` | Either `B_WIN_TYPE_NORMAL` or `B_WIN_TYPE_ARENA`. This isn't accessed by any battle scripts, but it's stored here nonetheless, being used by a lot of UI-related battle engine code. The Battle Arena has unique dialog graphics for showing the Mind/Skill/Body scoreboard mid-battle, and this value determines whether any of the windows and other UI resources for those are loaded. |
| 0x25 | `multiplayerId` | The player's multiplayer ID (i.e. which of the up to four linked consoles they're on). This isn't accessed by any battle scripts, but it's stored here nonetheless, being used by lots of C code. |
| 0x26 | `specialTrainerBattleType` | Set by `DoSpecialTrainerBattle`, and read by `HandleSpecialTrainerBattleEnd` to know what post-battle clean-up to perform (e.g. for the Battle Frontier or for Secret Bases). This isn't accessed by any battle scripts, but it's stored here nonetheless. |





[^tag-castform-form-change]: **Tag:** Castform form change

[^tag-getexp-latent-state]: **Tag:** `getexp` command latent state

[^tag-party-menu-latent-state]: **Tag:** Party menu latent state, when opened during battle

[^tag-DoFieldEndTurnEffects-latent-state]: **Tag:** `DoFieldEndTurnEffects` latent state

[^tag-HandleFaintedMonActions-latent-state]: **Tag:** `HandleFaintedMonActions` latent state

[^tag-HandleWishPerishSongOnTurnEnd-latent-state]: **Tag:** `HandleWishPerishSongOnTurnEnd` latent state

[^tag-TryDoEventsBeforeFirstTurn-latent-state]: **Tag:** `TryDoEventsBeforeFirstTurn`[^TryDoEventsBeforeFirstTurn] latent state

[^TryDoEventsBeforeFirstTurn]: Note that `TryDoEventsBeforeFirstTurn` only processes switch-in effects for abilities and held items at the start of a battle. Thereafter, these effects are processed by the `switchineffects` script command.

[^npc-ai-item-usage]: The code for handling item usage is a mess, frankly.

    When the player uses non-battle-specific items (e.g. Potions) during battle, those items' effects are processed wholly separately from the battle engine. The battle engine is told what item the player used, but unless that item happens to be battle-specific (e.g. a Poke Ball), the battle engine has no involvement in carrying out the item's effect, and no knowledge of what that effect is. This allows item usage code to be unified between field and battle use: Potions, Ethers, and the like do essentially the same thing both in and out of battle. However, it does give rise to some limitations, such as not being able to undo item usage during a Double Battle despite being able to undo other actions.
    
    The pertinent thing to know, of course, is that the vast majority of that item code only applies to the player's party specifically &mdash; not to NPC parties &mdash; which means that item use by NPC AI has to work *completely differently* from item use by the player. In essence, the battle engine *does* have to assume at least *some* responsibility for applying the effects of items used by NPC AI. This is why NPC AI item usage looks so very, very different from player item usage (e.g. with different constants for the item type being used).