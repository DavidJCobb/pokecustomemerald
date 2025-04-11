
# Roadmap

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

## Savedata

* Bitpack player inventory quantities more tightly, using struct transforms: most bag pockets have a max of 99 per item, but berries max out at 999 per berry type.

## Text

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

### Party menu

* Mini-sprites for battle-only forms (e.g. Castform transformations), as in Gen V

### PC

* Releasing a Pokemon should return its held item to the bag.
  * If there's insufficient bag space, then show a confirmation prompt warning that the held item will e lost.

### Pokedex

* Proportional scrollbar thumb
  * Similar to the reusable code we have, but since the Pokedex scrollbar thumb is styled with borders, we'll need more tiles to account for its various pieces.

## Research

### Battle engine

* All script commands
* The interplay between script commands and native code