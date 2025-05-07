
# Battle engine refactor

It's very hard to get a full sense of how the battle engine works. There are three distinct execution environments, which are *very* strongly coupled, and which rely heavily on global state:

* Native code
* Battle scripts, and the commands they execute
* Battle controllers

All three of these execution environments are capable of running latent code, which cannot store state using local variables; latent functions require a persistent storage area for their state. There's no clear delineation between the storage areas of these three execution environments: battle controllers will happily stash latent state in `gBattleStruct`, and native code stuffs some latent and persistent state in `gBattleScripting`.

On top of that, there are a handful of variables that exist to be used as operands, and these are the worst kind of global state: they serve a similar purpose to function arguments, but are reused all over the battle engine (including in parts of the battle engine that have leaked out into non-battle-related files, like `pokemon.c`), and so they're overwritten almost constantly with no clear rules as to when they might change. You can't reasonably anticipate the values of these variables on a moment-to-moment basis:

* `gActiveBattler`
* `gBattlerAttacker`
* `gBattlerTarget`
* `gEffectBattler`
* `gBattleScripting.battler` (the "script active" battler)

These issues combined mean that studying, understanding, and maintaining the battle engine is a daunting task. I simply can't keep the entire engine in my head all at once when looking at things, and without a clear idea of what gets used where and when, I sort of *have* to.

The best solution, I think, is an incremental refactor of the battle engine: chip away at the global state, identifying groups of common state and moving them into clearly delineated structs (nested or otherwise). Document which battle script commands require state which is set up by other battle script commands: document each command's preconditions, postconditions, and coupling. Slowly, gradually disentangle the engine.

## Global state refactoring

Broadly, the best way to refactor most global state will be to move it around. For example, the `lastTakenMove` and `lastTakenMoveFrom` fields on `gBattleStruct` both track the last move that a Pokemon was hit by (the former is needed to know the most recent move; the former tracks the most recent move from each other battler), but there are a dozen and a half unrelated fields between them. They could be grouped together within the struct and refactored as

```c
struct BattleStruct {
   // ... other stuff ...
   struct {
      MoveID most_recently_hit_with[MAX_BATTLERS_COUNT]; // formerly lastTakenMove
      MoveID most_recently_hit_by_attacker_with[MAX_BATTLERS_COUNT][MAX_BATTLERS_COUNT]; // formerly lastTakenMoveFrom
   } battler_hit_history;
   // ... other stuff ...
};
```

or, more radically but more cleanly,

```c
struct BattlerState_MostRecentlyHitWith {
   MoveID in_general;                      // formerly lastTakenMove[n]
   MoveID by_attacker[MAX_BATTLERS_COUNT]; // formerly lastTakenMoveFrom[n]
};

struct BattlerState {
   struct BattlerState_MostRecentlyHitWith most_recently_hit_with;
};

struct BattleStruct {
   // ... other stuff ...
   struct BattlerState battler_state[MAX_BATTLERS_COUNT];
   // ... other stuff ...
};
```

### Renaming structs

| Old name | New name | Rationale |
| :- | :- | :- |
| `DisableStruct` | `BattlerPersistentMoveState` | This struct describes battler state scoped to the battler's presence on the field (i.e. reset when the battler switches out or faints). |
| `ProtectStruct` | `BattlerTurnScopedState` | This struct describes battler state scoped to the current turn (i.e. reset at the end of the turn). |
| `SpecialStatus` | `BattlerActionScopedState` | This struct describes battler state scoped to the current action (i.e. reset at the end of the action). |

### Struct definitions

Even this refactor is a bit conservative. For example, some state is heap-allocated (`BattleStruct`) and some state is statically allocated (everything else); I didn't move any state across that particular boundary, even when it would've lead to better-organized code. Perhaps I should. But this is all just preliminary planning anyway.

```c
typedef u8 BattlerID;

struct BattlerState {
   struct {
      s8  mind_points;  // formerly BattleStruct::arenaMindPoints[n]
      s8  skill_points; // formerly BattleStruct::arenaSkillPoints[n]
      u16 initial_hp;   // formerly BattleStruct::arenaStartHp[n]
   } battle_arena;
   struct {
      BattlerID inflicted_by;   // formerly BattleStruct::wrappedBy[n]
      MoveID    inflicted_with; // formerly BattleStruct::wrappedMove[n]
   } bound_status;
   MoveID choice_locked_move; // formerly BattleStruct::choicedMove[n]
   struct {
      MoveID in_general;                      // formerly BattleStruct::lastTakenMove[n]
      MoveID by_attacker[MAX_BATTLERS_COUNT]; // formerly BattleStruct::lastTakenMoveFrom[n]
   } most_recently_hit_with;
   ItemIDGlobal queued_item_acquisition; // formerly BattleStruct::changedItems[n]
   struct {
      u8 party_slot_of_me;          // formerly BattleStruct::battlerPartyIndexes[n]
      u8 party_slot_of_replacement; // formerly BattleStruct::monToSwitchIntoId[n]
   } queued_switch_out;
   
   // The item ID of the battler's held item, if and only if the item has been used 
   // or destroyed. This is retained to facilitate the Recycle move. That said, we 
   // should probably store that state per party slot (player and enemy), not per 
   // battler ID.
   ItemIDGlobal used_held_item; // formerly BattleStruct::usedHeldItems[n]
};

// A battler's chosen turn action, action case, et cetera.
struct BattlerChoices {
   u8        move_index;  // formerly BattleStruct::chosenMovePositions[n]
   BattlerID move_target; // formerly BattleStruct::moveTarget[n]
   struct {
      u8    next_action_state; // formerly BattleStruct::stateIdAfterSelScript[n]
      bool8 finished;          // formerly BattleStruct::selectionScriptFinished[n]
   } selection_script;
};

// Similar to BattlerChoices, but with parameters that only apply to NPC AI. 
// That is: this isn't storage for things computed while an AI is thinking, 
// but rather for choices that the battle engine has to execute on behalf 
// of the AI (e.g. item usage, where player item use is handled outside of 
// the battle engine).
struct BattlerAIChoices {
   u8 item_effect_type; // formerly BattleStruct::AI_itemType[n]
   u8 item_effect_flags; // formerly BattleStruct::AI_itemFlags[n]
};

struct BattleStruct {
   struct { // BattleStruct::latent_state
      struct {
         u8 current_canceller; // formerly BattleStruct::atkCancellerTracker
      } AtkCanceller_UnableToUseMove;
      struct {
         BattlerID focus_punch_current_battler; // formerly BattleStruct::focusPunchBattlerId
      } CheckFocusPunch_ClearVarsBeforeTurnStarts;
      struct {
         u8        current_effect;  // formerly BattleStruct::turnEffectsTracker
         BattlerID current_battler; // formerly BattleStruct::turnEffectsBattlerId
      } DoBattlerEndTurnEffects;
      struct {
         u8 current_battlefield_side; // formerly BattleStruct::turnSideTracker
         u8 current_effect;           // formerly BattleStruct::turnCountersTracker
      } DoFieldEndTurnEffects;
      struct {
         u8        current_state;   // formerly BattleStruct::faintedActionsState
         BattlerID current_battler; // formerly BattleStruct::faintedActionsBattlerId
      } HandleFaintedMonActions;
      struct {
         u8        current_state;   // formerly BattleStruct::wishPerishSongState
         BattlerID current_battler; // formerly BattleStruct::wishPerishSongBattlerId
      } HandleWishPerishSongOnTurnEnd;
      struct {
         u8 turn_action_number_prior; // formerly BattleStruct::savedTurnActionNumber
      } RunTurnActionsFunctions;
      struct {
         u8        current_ability;          // formerly BattleStruct::switchInAbilitiesCounter
         BattlerID current_held_item_holder; // formerly BattleStruct::switchInItemsCounter
      } TryDoEventsBeforeFirstTurn;
      
      struct {
         u32 savedBattleTypeFlags;    // formerly BattleStruct::savedBattleTypeFlags
         void (*savedCallback)(void); // formerly BattleStruct::savedCallback
      } pre_init_multi_battle;
      struct {
         u8 overall_state;    // formerly BattleStruct::reshowMainState
         u8 health_box_state; // formerly BattleStruct::reshowHelperState
      } reshow_battle_screen;
      
      struct {
         u8 form_to_change_into; // formerly BattleStruct::formToChangeInto
      } queued_castform_form_change;
      struct {
         u8 chosen_ai_item_type[MAX_BATTLERS_COUNT]; // formerly BattleStruct::chosenItem
         u8 chosen_party_slot_to_switch_to[MAX_BATTLERS_COUNT]; // formerly BattleStruct::AI_monToSwitchIntoId
      } npc_ai;
      struct {
         struct {
            bool8 used_status_move_on_already_statused_target; // formerly BattleStruct::alreadyStatusedMoveAttempt
         } battle_arena;
         struct {
            u16 hp_on_switchout[NUM_BATTLE_SIDES]; // formerly BattleStruct::hpOnSwitchout
            u8  hp_scale;                          // formerly BattleStruct::hpScale
            u8  party_index;                       // formerly BattleStruct::scriptPartyIdx
         } switchout_messages;
      } features;
      struct { // BattleStruct::latent_state::script_commands
         struct {
            struct {
               BattlerID battler_id; // formerly BattleStruct::expGetterBattlerId
               u8        party_slot; // formerly BattleStruct::expGetterMonId
            } current_recipient;
            u8  possible_recipients; // formerly BattleStruct::sentInPokes. one bit per party slot.
            u16 total_exp;           // formerly BattleStruct::expValue
         } getexp;
         struct {
            u8 nickname[POKEMON_NAME_LENGTH + 1]; // formerly BattleStruct::caughtMonNick
         } trygivecaughtmonnick;
      } script_commands;
      struct { // BattleStruct::latent_state::controllers
         struct {
            struct {
               u8        ability_preventing_switchout; // formerly BattleStruct::abilityPreventingSwitchout
               BattlerID battler_preventing_switchout; // formerly BattleStruct::battlerPreventingSwitchout
               u8        previous_selected_party_slot; // formerly BattleStruct::prevSelectedPartySlot
            } party_menu_state;
         } player;
         struct {
            u8 battle_state; // formerly BattleStruct::wallyBattleState
            u8 moves_state;  // formerly BattleStruct::wallyMovesState
            u8 wait_frames;  // formerly BattleStruct::wallyWaitFrames
            u8 move_frames;  // formerly BattleStruct::wallyMoveFrames
         } wally;
      } controllers;
      
      // Unsure where precisely these latent values should go.
      
      BattlerID current_intimidating_battler; // formerly BattleStruct::intimidateBattler
      u8        current_dynamic_move_type;    // formerly BattleStruct::dynamicMoveType
      u8        synchronize_move_effect;      // formerly BattleStruct::synchronizeMoveEffect
   } latent_state;
   struct BattlerState     battler_state[MAX_BATTLERS_COUNT];
   struct BattlerChoices   battler_choices[MAX_BATTLERS_COUNT];
   struct BattlerAIChoices battler_ai_choices[MAX_BATTLERS_COUNT / NUM_BATTLE_SIDES];
   
   // These should probably be changed into function-local variables, but 
   // are kept here for now.
   struct {
      struct {
         MoveID possible_moves[PARTY_SIZE * MAX_MON_MOVES];
      } assistattackselect;
   } scratch_state;
   
   struct {
      s8 current_turn; // formerly BattleStruct::arenaTurnCounter
      
      // These next two values are bitmasks -- one per party member -- indicating the 
      // referee's decisions as to which side lost.
      u8 defeated_player_pokemon; // formerly BattleStruct::arenaLostPlayerMons
      u8 defeated_enemy_pokemon;  // formerly BattleStruct::arenaLostOpponentMons
   } battle_arena_state;
   struct {
      u8 approach_counter;   // formerly BattleStruct::safariGoNearCounter
      u8 ball_throw_counter; // formerly BattleStruct::safariPkblThrowCounter
      u8 escape_factor;      // formerly BattleStruct::safariEscapeFactor
      u8 catch_factor;       // formerly BattleStruct::safariCatchFactor
   } safari_zone_state;
   
   struct {
      u8 window_type; // formerly BattleScripting::windowsType // B_WIN_TYPE_...
      u8 linkBattleVsSpriteId_V; // The letter "V" // moved from BattleScripting
      u8 linkBattleVsSpriteId_S; // The letter "S" // moved from BattleScripting
   } graphics_ui_state;
   
   u8 wildVictorySong;
   u8 moneyMultiplier;
   u8 battlerPartyOrders[MAX_BATTLERS_COUNT][PARTY_SIZE / 2];
   u8 runTries;
   u8 stringMoveType;
   u8 absentBattlerFlags;
   u8 palaceFlags; // First 4 bits are "is <= 50% HP and not asleep" for each battler, last 4 bits are selected moves to pass to AI
   u8 field_93; // related to choosing pokemon?
   bool8 anyMonHasTransformed;
   u8 givenExpMons; // Bits for enemy party's Pokémon that gave exp to player's party.
   union {
     struct LinkBattlerHeader linkBattlerHeader;
     u32 battleVideo[2];
   } multiBuffer;
   bool8 overworldWeatherDone;
   struct BattleTvMovePoints tvMovePoints;
   struct BattleTv tv;
   
   u8 multiplayerId; // formerly BattleScripting::multiplayerId
   
   // deleted fields:
   //  - castformPalette
};

// Various globals coalesced together.
struct BattlerStateNonHeap {
   struct {
      struct {
         u8 summary_screen_task_id; // formerly gBattlerStatusSummaryTaskId[n]
      } controllers;
   } latent_state;

   struct {
      s32       damage; // formerly gBideDmg[n]
      BattlerID target; // formerly gBideTarget[n]
   } bide;
   struct {
      // NOTE: Consider moving into BattlerChoices instead?
      u8        chosen_action;    // formerly gChosenActionByBattler[n]
      MoveID    chosen_move_id;   // formerly gChosenMoveByBattler[n]
      const u8* selection_script; // formerly gSelectionBattleScripts[n]
   } choices;
   struct BattleEnigmaBerry enigma_berry; // formerly gEnigmaBerries
   struct {
      BattlerID hit_by_battler;      // formerly gLastHitBy[n]
      u8        hit_by_type;         // formerly gLastHitByType[n]
      MoveID    chosen_move;         // formerly gLastMoves[n]
      MoveID    chosen_printed_move; // formerly gLastPrintedMoves[n]
      MoveID    result_move;         // formerly gLastResultingMoves[n]
      MoveID    used_landed_move;    // formerly gLastLandedMoves[n]
   } most_recently;
   struct {
      u8 action; // formerly gActionSelectionCursor[n]
      u8 move;   // formerly gMoveSelectionCursor[n]
   } selection_cursor_positions;
   u8  sprite_id;               // formerly gBattlerSpriteIds[n]
   u32 status3;                 // formerly gStatuses3[n]
   u32 transformed_personality; // formerly gTransformedPersonalities[n]
   
   // This field is only checked if STATUS2_MULTIPLETURNS or STATUS2_RECHARGE. A 
   // "pointless" union seems like a decent enough way to express that.
   union {
      MoveID multiple_turn_move;
      MoveID recharging_move;
   }; // formerly gLockedMoves[n]
};
extern struct BattlerStateNonHeap gBattlerStates[MAX_BATTLERS_COUNT];
/*
   Globals to delete and redirect to instances of the above struct:
   
    - gActionSelectionCursor
    - gBattlerSpriteIds
    - gBattlerStatusSummaryTaskId
    - gChosenActionByBattler
    - gChosenMoveByBattler
    - gBideDmg
    - gBideTarget
    - gEnigmaBerries
    - gLastHitBy
    - gLastHitByType
    - gLastLandedMoves
    - gLastMoves
    - gLastPrintedMoves
    - gLastResultingMoves
    - gLockedMoves
    - gMoveSelectionCursor
    - gSelectionBattleScripts
    - gStatuses3
    - gTransformedPersonalities
*/

// NOTE: The members of this struct have hard-coded offsets
//       in include/constants/battle_script_commands.h
struct BattleScripting {
   // Battle configuration and state which needs to be easily visible to scripts.
   u8 battle_style;
   u8 special_trainer_battle_type; // OW scripts need to be able to set this before a battle starts
   
   // State set by native code when invoking a script, and forwarded by that script to 
   // other places.
   u8 animation_args[2]; // formerly animArg1 and animArg2
   
   // Latent state for script commands. Often initialized by 
   // battle scripts before invoking those commands (directly 
   // or indirectly).
   struct {
      struct {
         u8 current_turn; // formerly animTurn
         u8 targets_hit;  // formerly animTargetsHit
      } attackanimation; // also tampered with by removelightscreenreflect
      struct {
         u8 state; // formerly drawlvlupboxState
      } drawlvlupbox;
      struct {
         u8 state; // formerly getexpState
      } getexp;
      struct {
         u8 state; // formerly moveendState
      } moveend;
      struct {
         bool8 already_played; // formerly statAnimPlayed // init'd by scripts
      } playstatchangeanimation;
      struct {
         u8 state; // formerly learnMoveState
      } yesnoboxlearnmove_or_cancel;
   } script_command_latent_state;
   
   u8 current_stat_change; // formerly statChanger // set by scripts; read by several script commands
   
   struct {
      s32 pain_split_hp;      // scratch variable for scripts
      s32 queued_bide_damage;
      u16 triple_kick_power;  // scratch variable for scripts
      
      // Used by BattleScript_MultiHitLoop, with the intention that it be initialized 
      // (by scripts that jump there) to whatever move effect a multi-hit move should 
      // have. The loop is shared by many moves that have a variable number of hits.
      u8 current_multi_hit_effect; // formerly multihitMoveEffect
      
      // Used by BattleScriptFirstChargingTurn, with the intention that it be initial-
      // ized (by scripts that jump there) to whatever multi-string index a two-turn 
      // charging move should use to print the charge-up message.
      u8 two_turn_move_charging_string_index; // formerly twoTurnsMoveStringId
      
      // A buffer used to store the stringified hit count. The `initmultihitstring` 
      // script command generates the text, and then scripts eventually copy the 
      // string data to gBattleTextBuff1. The hit count is only available at the 
      // start of multi-hit processing, but is only displayed at the end, so buffering 
      // the text here at the start will guarantee that it doesn't get clobbered if 
      // anything else buffers to gBattleTextBuff1... but it doesn't look like anything 
      // ever actually *would* buffer there, so theoretically, we don't actually need 
      // to keep this buffer. We should look into removing it.
      u8 multi_hit_count_string[6]; // formerly multihitString
   } move_state;
   
   struct {
      BattlerID battler_with_ability; // set by jumpifability
      
      union { // formerly gBattleCommunication[0]
         bool8 is_battler_fainted;              // VARIOUS_GET_BATTLER_FAINTED
         bool8 is_running_impossible;           // VARIOUS_IS_RUNNING_IMPOSSIBLE
         bool8 should_print_palace_flavor_text; // VARIOUS_PALACE_FLAVOR_TEXT
      };
   } script_command_results;
   
   
   
   
   u8 damage_multiplier; // formerly dmgMultiplier
   
   u8 battler;
   
   // TODO: This isn't even battle state. It's set during the common function that 
   // handles all level-ups (including in battles), but it's only used when processing 
   // item effects that heal level-up HP. Specifically, Rare Candies are flagged as not 
   // only increasing a Pokemon's level, but also healing an amount of HP by which the 
   // Pokemon's max HP has increased. You can't use Rare Candies outside of battle.
   //
   // We should move this state somewhere else, but finding a better home for it would 
   // require digging into the global state related to item usage.
   u8 levelUpHP;
   
   // Additions:
   u8 current_move_effect; // formerly gBattleCommuncation[MOVE_EFFECT_BYTE]
} gBattleScripting;
/*
   Naming conventions changed to snake-case for consistency with the above 
   refactor. Fields reordered by category.
   
   We can also remove `pursuitDoublesAttacker` and the unused `pursuitdoubles` 
   script command, its only user.
*/

struct BattleCurrentTurn {
   struct {
      u8        actions[MAX_BATTLER_COUNT];  // replaces gActionsByTurnOrder
      BattlerID battlers[MAX_BATTLER_COUNT]; // replaces gBattlersByTurnOrder
   } turn_order;
   struct {
      u8 current_turn_action;    // replaces gCurrentTurnActionNumber
      u8 current_action_func_id; // replaces gCurrentActionFuncId
   } action_exec_latent_state;
} gCurrentTurn;
```


## Script engine refactoring

The script engine is a rat's nest. Is there anything we can do about that?

At minimum, I should invest effort into documenting...

* Which commands are latent, versus which complete instantly.
* Which commands wait for battle controllers to become idle.
* Which commands dispatch to battle controllers.
* Which commands modify `gActiveBattler`.
* The preconditions of each command (i.e. what variables it expects to have already been set up for it).

But can we also alter the commands? Can we combine tightly-coupled commands together?

(In particular, the game defines commands up through 0xF8, which means that we don't have a lot of breathing room for implementing new move effects. In fact, pokeemerald-expansion had to add a `callnative` command and start leaning heavily on that, because they just ran out of command IDs.)

* `jumpifaffectedbyprotect` seems unused. Can we remove it?

* `removeattackerstatus1` seems unused, and the implementation is wrong (it doesn't sync via battle controllers). Can we remove it?

* Multiple script commands have wholly separate "state" fields stored in `gBattleScripting`. This is because battle scripts are a stack: scripts can invoke other scripts (including indirectly, e.g. if a script runs a script command that invokes a script, or if a script runs a script command that calls a native function that invokes a script, and so on). Theoretically, one could refactor this so that script command latent state is stored with the current call stack frame, rather than being kept on `gBattleScripting` and other globals. As a bonus, the latent states could be stored in a union, so that each stack frame only needs to reserve enough room for whichever command has the largest amount of state.

  * By way of comparison: Game Freak's approach gives each script command [that needs latent state] its own dedicated state, and this is fine as long as script commands that need latent state are never reentrant.

* Rename `negativedamage` to `prepabsorbfromdamagedealt`. It sets `gBattleMoveDamage` to either `-(gHpDealt / 2)` or, if that value is zero, to `-1` as a sentinel value; it's clearly intended for attacks wherein the attacker regains 50% as much HP as they took from the enemy.

  * Actually, why not ditch the `negativedamage` command entirely? In its place, we can add a `DMG_CALC_ABSORPTION` constant for use with `manipulatedamage`.

* `waitanimation` and `waitstate` have exactly identical behavior, waiting for all battle controllers to become idle. They differ only in their semantics. Would it be better or worse to just merge them?

  * Some battle scripts are mismatched, e.g. `BattleScript_HitFromAtkAnimation` using `waitstate` after `playanimation`, so the semantics don't seem to amount to much...

* `healthbarupdate` and `datahpupdate` are always run in tandem with one another. The only reason they're separate commands is because each has to dispatch a message to battle controllers, with a wait between them; to unify these commands, we'd have to give them latent state. That might be worthwhile, combining them into an `updatehp` command.

* `effectivenesssound` and `hitanimation` are always paired except for one Memento edge-case and for any scripts that deal with attacks missing, in which case only the former is run. We could combine them, but should we? They seem quite okay being separate.

* Can we combine `sethail`, `setrain`, `setsandstorm`, and `setsunny` into a single `setweather` command that takes an argument?

* Can we combine `setsemiinvulnerablebit` and `clearsemiinvulnerablebit` into `modsemiinvulnerablebit`? Alternatively, can we make a general-purpose function for modifying `STATUS3_...` bits, allowing us to also get rid of `setminimize`, `trysetroots`, `setseeded`, `setalwayshitflag`, `setyawn`, and `setcharge`?

  * The latter few commands are bit more complex, but if we switch-case on an enum, we can handle them as they need.
  
  * We could define an underlying `lu_impl_trymodstatus3 <status> <operation> [<jump-ptr>]`, without an ASM macro, wherein `<jump-ptr>` is only present and consumed if `<operation>` has the high bit set; and then we could define ASM macros to transparently replace the original commands, such that `setminimize` would be `lu_impl_trymodstatus3 MINIMIZE 0x01` while `trysetroots <jump-target>` would be `lu_impl_trymodstatus3 ROOTS 0x81 <jump-target>`

* If we replace `cureifburnedparalysedorpoisoned` with a `curestatus1` command that takes a flags-mask argument of statuses to cure, then we could potentially use it for new moves. Hold off until we have a new move that would benefit from this.

* I feel like `jumpifconfusedandstatmaxed` could be removed, its usages in scripts replaced with `jumpifstatus2` and some jump command related to stats being at min/max (I know something exists to let us check that; check the battle scripts, not the command list).

### Fainting

Fainting occurs in the following scenarios, with the following scripts:

**Attacker faints**

```
BattleScript_FaintAttacker::
	playfaintcry BS_ATTACKER
	pause B_WAIT_TIME_LONG
	dofaintanimation BS_ATTACKER
	cleareffectsonfaint BS_ATTACKER
	printstring STRINGID_ATTACKERFAINTED
	return
```

**Target faints**

```
BattleScript_FaintTarget::
	playfaintcry BS_TARGET
	pause B_WAIT_TIME_LONG
	dofaintanimation BS_TARGET
	cleareffectsonfaint BS_TARGET
	printstring STRINGID_TARGETFAINTED
	return
```

**Battle Arena: player wins**

```
@BattleScript_ArenaDoJudgment::
@...
	printstring STRINGID_DEFEATEDOPPONENTBYREFEREE
	waitmessage B_WAIT_TIME_LONG
	playfaintcry BS_OPPONENT1
	waitcry BS_ATTACKER
	dofaintanimation BS_OPPONENT1
	cleareffectsonfaint BS_OPPONENT1
@...
```

**Battle Arena: player loses**

```
@BattleScript_ArenaJudgmentPlayerLoses::
@...
	printstring STRINGID_LOSTTOOPPONENTBYREFEREE
	waitmessage B_WAIT_TIME_LONG
	playfaintcry BS_PLAYER1
	waitcry BS_ATTACKER
	dofaintanimation BS_PLAYER1
	cleareffectsonfaint BS_PLAYER1
@...
```

**Battle Arena: draw**

```
@BattleScript_ArenaJudgmentDraw::
@...
	printstring STRINGID_TIEDOPPONENTBYREFEREE
	waitmessage B_WAIT_TIME_LONG
	playfaintcry BS_PLAYER1
	waitcry BS_ATTACKER
	dofaintanimation BS_PLAYER1
	cleareffectsonfaint BS_PLAYER1
	playfaintcry BS_OPPONENT1
	waitcry BS_ATTACKER
	dofaintanimation BS_OPPONENT1
	cleareffectsonfaint BS_OPPONENT1
@...
```

We can see that `dofaintanimation` and `cleareffectsonfaint` are always called together. Theoretically, those could be combined into a single `dofaint` command; however, each dispatches a different battle controller message, so we'd need latent state in order to know what command to dispatch.

The timings of each action vary as well. Ordinary fainting plays the faint cry, waits, animates, and prints a message; whereas Arena outcomes print a message, wait, play the faint cry, and then animate. We could potentially group playing the faint cry and optionally waiting into the same command as well:

```
dofaint <subject> <bool:pause_after_cry>

@
@ dofaint <subject> 0
@ replaces:
@    playfaintcry <subject>
@    waitcry <subject>
@    dofaintanimation <subject>
@    cleareffectsonfaint <subject>
@
@ dofaint <subject> 1
@ replaces:
@    playfaintcry <subject>
@    pause B_WAIT_TIME_LONG
@    dofaintanimation <subject>
@    cleareffectsonfaint <subject>
@
```

This allows us to replace three command definitions (`playfaintcry`, `dofaintanimation`, `cleareffectsonfaint`) with one (`dofaint`). It also means that instead of having two tightly-coupled script commands, we have a single script command: that is, instead of having one thing pretending to be two separate things, we just have *one thing*.

The catch, of course, is that `playfaintcry`, `dofaintanimation`, and `cleareffectsonfaint` all send battle controller commands. When these are separate commands, they don't need to be latent: they can be fire-and-forget. Combining them, however, means we'll need a latent state counter to know what step of the process we're on.


## Declarative move effects

This sounds a little far-fetched, but it sure is tempting...



## Battle controller and communication refactor

* Rename `gBattleBufferA` to `gBattleComm_EngineToControllers`. Rename `BUFFER_A` to `MSGBUFFER_ENGINE_TO_CONTROLLERS`.
* Rename `gBattleBufferB` to `gBattleComm_ControllersToEngine`. Rename `BUFFER_B` to `MSGBUFFER_CONTROLLERS_TO_ENGINE`.

