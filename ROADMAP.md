
# Roadmap

## Needed polishing

* The pick-species menu takes *way* too long to sort the species list when no filters are applied, causing a very visible lag-spike. We need to implement an "async sort" function, so we can divide the sorting work over multiple frames when there are many, many species to sort.
  * We should show a loading indicator in place of the listing while a sort is ongoing.
  * The menu should remain operable while a sort is ongoing, but the user shouldn't be able to move the cursor into the listing pane until the sort is complete.
    * The "Choose Pokemon >>" link should be shown in grey while a sort is ongoing.
* If you change whichever of the three starter species the rival ends up using, then their parties should be altered to use that species, dynamically computing the appropriate stats, evolutions, etc., based on level.

## Characters

* Skin and outfit color variations for NPCs (overworld and trainer sprites)

### Specific NPCs

* Backport new designs for minor characters from Omega Ruby and Alpha Sapphire
  * Courtney (Magma Admin)
  * Matt (Aqua Admin)
  * Shelley (Aqua Admin)
  * Tabitha (Magma Admin)

## Custom Game Options

* Infinite Rare Candies from game start
* Infinite stat vitamins from game start
* Mono-type and dual-type runs
  * Enabled (Yes/No)
  * Type 1
  * Type 2
    * None
    * Any
  * Dual-Type Behavior (Require Both Types/Require Either Type)
  * Pokemon with the wrong type (Always Disallow/Allow Half-Matches/Force Half-Matches/Always Force)
    * "Control whether and how you can use Pokemon whose types don't match the ones listed here."
    * Always Disallow: "You cannot use Pokemon unless their types exactly match the types chosen here."
    * Allow Half-Matches: "If you choose two types, and a Pokemon has only one of them, then you can use that Pokemon."
    * Force Half-Matches: "If you choose two types, and a Pokemon has only one of them, then its other type will be replaced with the other type (though only when you use it)."
    * Always Force: "You can use any Pokemon species. Their types will be replaced with the types you choose here (though only when you use them)."
  * Player moves of other types (Unchanged/Force to Type 1/Force to Type 2)
* Nuzlocke
  * Slain Pokemon behavior (Graveyard/Delete)
  * Encounter Limit (per area)
    * Enable/Disable
    * Gift Pokemon behavior (Free/Limits/Disable All Gifts)
    * Shiny Exception (None/Replace Prior/Always Allow)
  * Dupes Clause (Enable/Disable)
    * Dupes Clause Shiny Exception (None/Replace Prior/Always Allow)
  * Deactivation (Never/After Champion/After Rayquaza/After All Legendaries)
* Replace the three starters.
  * Dynamically alter the rival's starter to match the replacements.

### Custom game option behavior

* Allow the player to change options mid-playthrough
  * Save the fact that they've made these changes
  * Only allow changing an option when doing so would have an effect
    * Changing "Start game with running shoes" should be disabled after starting a playthrough
    * Changing the starter options should be disabled after obtaining a starter
    * NOTE: The CGO menu must know where it was opened from (e.g. New Game/title; in-game Options), in order to enforce this only when it should.

## Engine

### Items

* Investigate separating item IDs into per-bag-pocket ID ranges. This is something we'll want a compiler plug-in to do, but it'd allow bitpacking the player's inventory (which is stored per-pocket) within savedata even more tightly.
  * Depending on how far we can take this, it may also enable expanding the list of Poke Balls without needing all ball item IDs to be consecutive. This would make it considerably easier to add Poke Balls. However, that'll depend on things like redoing how the "mon data" for Poke Balls is stored, among other things.

### Scripting (overworld)

* Investigate making multiple-choice menus scriptable, such that the script can decide the quantity and text of the selectable options.

## Features

* Ball Capsules
  * Gen IV only stored up to 12 designs, and let you assign these to a Pokemon as desired. It wasn't "one design for every Pokemon," so it might actually be viable to implement.
* Overworld follower Pokemon

### Battles

* Fairy type
* Physical/special split

## Pacing

### Battles

* Concurrent animations and text for stat changes.
  * Separate battle script commands trigger the animation and the text, and both work by broadcasting messages to the battle controllers and waiting for a response. Changing this flow risks breaking vanilla link compatibility, and would require a unified message for battle controllers (i.e. "message, animation, and optional minimum duration").
* Health bar changes should animate in proportion with the damage done. When a Lv. 100 Mewtwo annihilates some bug in Viridian Forest, the health bar should drain rapidly both to emphasize the sheer power of the hit and to avoid keeping players waiting as they swat trivial foes out of their way.
* How difficult would it be to merge the messages for passive (i.e. non-damaging) weather into the player's choices, so there's one less dialog box to click through? (E.g. "Rain continues to fall. What should COMBATANT do?" with the rain animation playing continuously while the player makes their selection.)
  
#### Coalesced battle text

This will require thorough investigation of the battle script engine as well as changes to battle controllers, and will almost certainly break vanilla link compatibility.

* Move use
  * ATTACKER used MOVE, but it failed!
  * ATTACKER used MOVE, but the attack missed!
  * ATTACKER used MOVE, but it doesn't affect TARGET...
  * ATTACKER used MOVE. It's not very effective...
  * ATTACKER used MOVE. It's super effective!
* Move use (multi-hit)
  * ATTACKER used MOVE, but only hit TARGET one time.
  * ATTACKER used MOVE, and hit TARGET NUMBER times!
* Move use (status inflicted)
  * ATTACKER used MOVE, and STATUSed TARGET!
  * ATTACKER used MOVE. It's super effective, and it STATUSed TARGET!
  * ATTACKER used MOVE. It's not very effective, but it STATUSed TARGET!
  * ATTACKER used MOVE, but it failed: TARGET is already STATUSed.
    * Only if the move does nothing but inflict a status.
    * Overrides the "it missed" case. If Thunder Wave misses but also wouldn't have worked anyway, because the target is already paralyzed, we want to tell the user about the latter, not the former.
* Move use (self-healing)
  * ATTACKER used MOVE, and restored ATTACKER_PRONOUN_POSSESSIVE health.
  * ATTACKER used MOVE, but it failed: ATTACKER_PRONOUN_POSSESSIVE HP is already full![^battle-message-hp-is-full]
  * ATTACKER used MOVE, and cured ATTACKER_PRONOUN_REFLEXIVE of STATUS.
* Move use (miscellaneous)
  * ATTACKER used MOVE... and TARGET endured the hit!
    * Complication: You can endure multi-hit moves.
  * ATTACKER used MOVE, but TARGET protected TARGET_PRONOUN_REFLEXIVE!
  * ATTACKER used MOVE, but TARGET_PRONOUN_POSSESSIVE Substitute took the hit for TARGET_PRONOUN!
  * ATTACKER used MOVE, but hurt ATTACKER_PRONOUN_REFLEXIVE in ATTACKER_PRONOUN_POSSESSIVE confusion!
  * ATTACKER and TARGET fainted!
    * Moves that damage both the user and the target, e.g. Explosion and anything with recoil.
* Move effects
  * ATTACKER's STAT and OTHER_STAT sharply rose!
  * ATTACKER's STAT, OTHER_STAT, and THIRD_STAT rose!
  * All of ATTACKER's stats rose!
* System
  * COMBATANT gained NUMBER XP, and grew to Lv. NUMBER!
  * COMBATANT and Foe COMBATANT WEATHER_DAMAGE_PHRASE.
    * "Gastly and Foe Misdreavus are buffeted by the sandstorm."
* Held items
  * COMBATANT ate COMBATANT_PRONOUN_POSSESSIVE ITEM and restored NUMBER HP.
  * COMBATANT ate COMBATANT_PRONOUN_POSSESSIVE ITEM and cured COMBATANT_PRONOUN_POSSESSIVE STATUS.
* Double battles
  * ENEMY_TRAINER send out COMBATANT and COMBATANT!
  * Multi-target moves
    * ATTACKER used MOVE. It's super effective against ONE_TARGET!
    * Attacker used MOVE. It's not very effective against ONE_TARGET.
    * ATTACKER used MOVE. It hit ONE_TARGET, but it doesn't affect TWO_TARGET.
    * ATTACKER used MOVE. It's super effective against ONE_TARGET, but not very effective against TWO_TARGET.
    * TARGET and TARGET fainted!
  * COMBATANT, COMBATANT, and COMBATANT WEATHER_DAMAGE_PHRASE.
  * All Pokemon on the field WEATHER_DAMAGE_PHRASE.
* Persistent changes to a battler
  * NAME forgot MOVE, and learned REPLACEMENT_MOVE.
* Specific moves
  * Destiny Bond
    * ATTACKER used Destiny Bond. ATTACKER_PRONOUN_SUBJECT_IS trying to take ATTACKER_PRONOUN_POSSESSIVE foe down with ATTACKER_PRONOUN!
      * "Tangela used Destiny Bond. He's trying to take his foe down with him!"
  * Dream Eater
    * ATTACKER used Dream Eater, but it failed: TARGET is still awake.
  * Perish Song
    * COMBATANT and COMBATANT's Perish count fell to NUMBER!
      * Only if both combatants are affected and have the same count, i.e. no one switched out.
  * Rest
    * ATTACKER used Rest, but it failed: PRONOUN_SUBJECT_IS already asleep.
      * Can occur if Rest is used via Sleep Talk.
    * ATTACKER used Rest, but it failed: SLEEP_FAILURE_MESSAGE
      * There's a string table used near `jumpifcantmakeasleep` to handle cases where an enemy can't fall asleep, e.g. Uproar (in the absence of Soundproof), Insomnia, and Vital Spirit.
    * ATTACKER used Rest and fell asleep, but ATTACKER_PRONOUN_POSSESSIVE HP is already full.
      * Actually, would the user still fall asleep? We need to investigate `trysetrest`.
  * Spit Up
    * ATTACKER used SPIT UP, but failed to SPIT UP a thing!
  * Swallow
    * ATTACKER used SWALLOW, but failed to SWALLOW a thing!

[^battle-message-hp-is-full]: The separate "NAME's HP is full!" error text in vanilla is `STRINGID_PKMNHPFULL`, displayed by two battle scripts:
* BattleScript_AlreadyAtFullHp
  * Caller: BattleScript_EffectRestoreHp (generic handler for HP-restoring moves)
  * Caller: BattleScript_EffectRest
  * Caller: Three identical "recover based on sunlight" moves that use the same script in the same location:
    * BattleScript_EffectMorningSun
    * BattleScript_EffectSynthesis
    * BattleScript_EffectMoonlight
  * Caller: BattleScript_EffectSoftboiled
* BattleScript_WishButFullHp (branch in the Wish move handler, if the wish comes true)

### Text

* A "Very Fast" text speed option

## Savedata

* Bitpack player inventory quantities more tightly, using struct transforms: most bag pockets have a max of 99 per item, but berries max out at 999 per berry type.

## Text

* Instant text for substitution tokens
  * Goal: Make it so that speedrunners can use any name without losing time
  * Player names
  * Rival name
  * Pokemon species names (when variable)
  * Pokemon nicknames
* Automatic word-wrapping improvements
  * Investigate automatic hyphenation for simple cases
    * Only for words that are 6+ characters long
    * **`/(.)(ing)$/`:** "grabbing" -> "grab-bing"
    * **`/(ing)$/`:** "evolving" -> "evolv-ing" (lower-priority than the previous)
    * **`/(.{5,})(ution)/`:**" "revo-lution" but not "so-lution"; prefix must be 5+ chars long
* Decapitalization
  * Replace capitalization with coloration where appropriate
  * Investigate adding format codes (e.g. for species names, move names, etc.)
* Investigate support for more text formatting
  * Bold
  * Italics

### Battles

* Pronouns for Pokemon (i.e. "he" or "she" when appropriate, rather than always "it")
  * subject: he/she/they/it
  * object: him/her/them/it
  * possessive: his/hers/their/its
  * reflexive: himself/herself/themself/itself
  * subject-is: he's/she's/they're/it's

## UI

### Overworld

* Hotkey menu: Allow hotkeying multiple items, with a menu letting you pick which to use.
  * Also requires UI changes to the bag in order to manage hotkeys.
* Topbar
  * Implement as a feature, but disable or make optional for QoL.
  * This could show the current the health of the player's party lead, and the player's Run toggle state. For hacks with day/night cycles, it could show the current in-game time. We'd want the classic 6x6px font, so the topbar can be just a single tile high.
* When a Repel wears off, prompt the user to use another, if they have another of the same type. Display the quantity remaining as well.

#### Dialogue

* A UI widget to display the name of the speaking NPC.
* Unit portraits

### Clock set

* Show the exact time (HH:MM) rather than forcing players to rely on the clock hands alone.

### Party menu

* Mini-sprites for battle-only forms (e.g. Castform transformations), as in Gen V

### PC

* Releasing a Pokemon should return its held item to the bag.
  * If there's insufficient bag space, then show a confirmation prompt warning that the held item will e lost.

### Pokedex

* Proportional scrollbar thumb
  * Similar to the reusable code we have, but since the Pokedex scrollbar thumb is styled with borders, we'll need more tiles to account for its various pieces.

### Summary

* Replace the Emerald design with the FR/LG design, as this will offer a more flexible base.
  * (Keep the header row unchanged from Emerald; I like the colors on that better.)
  * (Keep the marking icons. Display them with the sprite, to the bottom left of that area of the screen, i.e. same Y-coordinate as the caught ball.)
* Modify the Battle Moves page:
  * When a move's details are open...
    * The move description doesn't need a label, so let's repurpose the "EFFECT" label: change it to "CATEGORY", display the physical/special icon to its right, and reduce the padding on the description box that visually joins it to the "EFFECT" label.
* Add a Contest Moves page back in.
  * Base it on the Battle Moves page.
  * Instead of "Power" and "Accuracy," show "Appeal" and "Jam," each stat as one row of eight hearts rather than two rows per stat and four hearts per row.
* Add a page for "Ribbons," and remove the equivalent page from the PokeNav.

### Vanilla bug fixes

#### Battle

* Do not allow Ralts to be shiny during Wally's cutscene battle.
* Do not allow Ralts to faint during Wally's cutscene battle.
* Track consumed items based on the Pokemon that consumed them, not the battle position that the Pokemon occupied at the time the item was consumed. This will get Recycle to behave as it does in modern battles, while indirectly resolving the only known mail corruption exploit in Emerald.
  * NOTE: This will have to depend on whether we're in a multiplayer link with a vanilla game.
  * NOTE: Arguably this should be a Custom Game option, since some "legacy" strategies may depend on the old behavior.
* If a Pokemon levels up while transformed, it will use its non-transformed stats for the remainder of the battle.
  * NOTE: Whether we apply fixed behavior for this will have to depend on whether we're in a multiplayer link with a vanilla game.

#### Overworld

* Dive glitch
* If a trainer catches the player in their line of sight while the player is on a tile that forces movement (e.g. fast currents in ocean routes), then when the battle ends, the player's forced movement should resume. In vanilla, it doesn't. (Cases: Swimmer Laurel and Swimmer Jack, on Route 134.)

#### UI

* Display Nature Power's accuracy as ??? or ---, since it depends on what move effect Nature Power calls into.
* All moves that always hit should be listed with Accuracy ---.

## Research

### Battle engine

* All script commands
* The interplay between script commands and native code