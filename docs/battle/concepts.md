
# Battle concepts

Battles are divided into **turns**, during which zero or more battlers will perform **actions**. When a battle ends, it has an **outcome**.

## Actions, action cases, and action selection states
<a name="actions"></a>

An **action** is a general *category of thing* that a participant in a battle can do, such as using a move, using an item, switching Pokemon, or running. An **action case** is a specific parameter supplied alongside an action, such as the move to use, the item to use, or the Pokemon to switch to.

The battle engine uses the `HandleTurnActionSelectionState` function to manage the task of prompting battle controllers for an action, prompting them for an action case if one is required, and waiting for all actions to be fully submitted. It also supports:

* Useful sentinel values for "backing out." For example, if a battle controller submits `B_ACTION_USE_ITEM`, it will be prompted for an action case; and in response, it can submit an item ID, or it can submit zero to "back out" and be re-prompted for an action.

* Some actions or action cases may prompt the battle engine to run a **selection script** for the battler. For example, failing to run from a battle will run a variety of selection scripts with different UI messages; and attempting to run from a Battle Frontier or Trainer Hill battle will run a selection script prompting the player as to whether they want to forfeit the battle. The battle engine can decide which action selection state (see below) the battler should be placed in once the script finishes executing.

Each battler has an **action selection state**, which indicates where the battler and its controller are in the overall process of submitting an action and, if needed, an action case. The action selection state for some battler <var>n</var> is stored in <code>gBattleCommunication[<var>n</var>]</code>; the possible states are:

| State | Description |
| :- | :- |
| `STATE_TURN_START_RECORD` | Recorded Battle information will be captured, before advancing to the next state. |
| `STATE_BEFORE_ACTION_CHOSEN` | If this battler is absent, then action `B_ACTION_NOTHING_FAINTED` is auto-selected, and the controller advances to either `STATE_WAIT_ACTION_CONFIRMED_STANDBY` (in Multi Battles) or `STATE_WAIT_ACTION_CONFIRMED`. Otherwise, if the battler has `STATUS2_MULTIPLETURNS` or `STATUS2_RECHARGE`, then `B_ACTION_USE_MOVE` is auto-selected, and the controller advances to `STATE_WAIT_ACTION_CONFIRMED_STANDBY`. Otherwise, the controller is asked to choose an action, and we advance to `STATE_WAIT_ACTION_CHOSEN`. |
| `STATE_WAIT_ACTION_CHOSEN` | Battle engine is waiting for a battle controller to select an action. When that selection is handled, the controller will be advanced to either `STATE_WAIT_ACTION_CASE_CHOSEN`, `STATE_WAIT_ACTION_CONFIRMED_STANDBY`, or `STATE_SELECTION_SCRIPT`. |
| `STATE_WAIT_ACTION_CASE_CHOSEN` | Battle engine is waiting for a battle controller to select an action case. |
| `STATE_WAIT_ACTION_CONFIRMED_STANDBY` | The battle controller has finalized its selection, so the battle engine is sending "link standby" messages to it. |
| `STATE_WAIT_ACTION_CONFIRMED` | Battle engine is waiting for all battle controllers to have handled the messages from the previous state. |
| `STATE_SELECTION_SCRIPT` | A battle controller chose an action which triggered the execution of a battle script. The controller will be in this state while the script executes, and will be routed to a different state (decided at the same time as the script to execute) when the script finishes. |
| `STATE_WAIT_SET_BEFORE_ACTION` | 
| `STATE_SELECTION_SCRIPT_MAY_RUN` | Same as `STATE_SELECTION_SCRIPT`, except that the battle controller may send `B_ACTION_NOTHING_FAINTED` in order to signal a decision to run (i.e. as if they had chosen `B_ACTION_RUN`). |

The actions are:

| Action | Description |
| :- | :- |
| `B_ACTION_USE_MOVE` |
| `B_ACTION_USE_ITEM` |
| `B_ACTION_SWITCH` | The battle controller wishes to switch Pokemon. They must submit an action case; submitting `PARTY_SIZE` as the action case indicates a desire to cancel and choose a different action. |
| `B_ACTION_RUN` | <p>During a Link Battle, indicates a decision to forfeit; either of outcome flags `B_OUTCOME_LOST` or `B_OUTCOME_WON` will be set depending on whether someone on the player's side made the decision. During all other battles, indicates a decision to flee (whether made by the player or an opposing Wild Pokemon).</p><p>If a battle controller submits this action during a trainer battle, the decision will be intercepted by `HandleTurnActionSelectionState`, which applies all of the logic for altering the player's decision before punting them back to `STATE_BEFORE_ACTION_CHOSEN`. Specifically, battle scripts will be queued to run for either allowing the player to forfeit (if they're in the Battle Frontier or Trainer Hill), refusing their decision (if they're in any other trainer battle), or trapping them (if they can't escape the Wild Pokemon). Unless those battle scripts end the battle on their own, the player will thus be given the chance to choose a different action.</p> |
| `B_ACTION_SAFARI_WATCH_CAREFULLY` | Safari Zone: The Wild Pokemon is choosing not to do anything. |
| `B_ACTION_SAFARI_BALL` | Safari Zone: The player is throwing a Safari Ball. |
| `B_ACTION_SAFARI_POKEBLOCK` | Safari Zone: The player is throwing a Pokeblock. |
| `B_ACTION_SAFARI_GO_NEAR` | Safari Zone: The player is approaching the Wild Pokemon. |
| `B_ACTION_SAFARI_RUN` | Safari Zone: The player has chosen to flee. Bypasses all of the normal logic involved in running from a battle, guaranteeing that the player escapes successfully. |
| `B_ACTION_WALLY_THROW` | Wally is throwing a Poke Ball. Used in the Pokemon catching cutscene. |
| `B_ACTION_EXEC_SCRIPT` |
| `B_ACTION_TRY_FINISH` | Sentinel value used internally. |
| `B_ACTION_FINISHED` | Sentinel value used internally. |
| `B_ACTION_CANCEL_PARTNER` | Sentinel value used for communications between the battle engine and battle controllers. During a Double Battle, a controller that has chosen an action for its lefthand Pokemon can submit this sentinel value as its righthand Pokemon's action, in order to undo that choice and request to make a new choice for its lefthand Pokemon. |
| `B_ACTION_NOTHING_FAINTED` | Sentinel value used internally. Auto-selected for absent battlers (i.e. this action is what a vacant position on the battlefield will do).
| `B_ACTION_NONE` | Sentinel value for when a battler hasn't yet chosen an action.

## Battle TV

Some in-game TV programs will go on the air depending on the player's actions during a match. These programs all rely on three hooks:

* `BattleTv_SetDataBasedOnString(u16 stringId)` is called whenever a battle message is displayed. It's called by the battle controllers when they respond to the `CONTROLLER_PRINTSTRING` message.
* `BattleTv_SetDataBasedOnMove(u16 move, u16 weatherFlags, struct DisableStruct*)` is called when any move animation plays. It's called by the battle controllers when they respond to the `CONTROLLER_MOVEANIMATION` message.
* `BattleTv_SetDataBasedOnAnimation(u8 animationId)` is called when any battle animation plays. It's called by the battle controllers when they respond to the `CONTROLLER_BATTLEANIMATION` message.

These hooks do not, on their own, provide all of the information that the Battle TV needs. For example, the Battle Seminar show may go on the air if the player uses a not-very-effective move; this relies on the `CONTROLLER_PRINTSTRING` hook, but has no clean way to know what move the player used. To that end, it relies on global state: `gBattleMons[gBattlerAttacker].moves[gMoveSelectionCursor[gBattlerAttacker]]`.

## Outcome


