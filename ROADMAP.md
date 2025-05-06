
# Roadmap

## Needed polishing

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

Flow changes:

* Change the flow for starting a new game.
  * Enhanced
    * All optional QOL changes are enabled.
  * Vanilla+
    * All optional QOL changes are disabled.
  * Nuzlocke
    * All optional QOL changes are enabled, unless they make the game aesier. Nuzlocke options are also enabled, using Graveyard boxes. The encounter limit (first obtained) and dupes clause are both neabled, as are shiny exceptions, and gifts count as encounters.
    * The player is shown all to-be-used options and given an opportunity to tweak them.
  * Custom Game
    * Open the full CGO menu.
* Make it so that the ability to adjust custom game options during a playthrough is, itself, a custom game option.
  * Don't allow opening the CGO menu via Options if Options is opened via the main menu.
  * Don't allow opening the CGO menu via Options if CGO options can't be adjusted during a playthrough.
* Add quick options to the CGO menu to set options to match generations, e.g. setting "Gen 4" would change the available options to match Gen 4 game mechanics as much as is possible. (We have a `SetCustomGameOptionsPerGeneration` function to help with this.)

Add new options:

* Battling
  * Cap player party levels
    * Constant maximum
      * "The player cannot train any Pokemon past this level. Pokemon obtained above this level will be reduced to this level."
      * *Obtained Pokemon have their level reduced* after *their met level is set.*
    * Dynamic maximum (None/Furthest Wild/Next Gym Leader)
      * Furthest Wild: "The player cannot train any Pokemon past the level of the strongest wild Pokemon in any location they've traveled to before."
      * Next Gym Leader: "The player cannot train any Pokemon beyond the level of the strongest Pokemon on the next Gym Leader's team. Once all Gym Leaders are defeated, Rival and Champion battles determine the cap."
  * Cap player party size
    * *If capped to size 1, we have to allow the player to always enter Double Battles, so that Tate & Liza isn't a softlock.*
  * Double Battles (Normal/Permissive/Forced/Disabled)
    * Normal: "You can enter Double Battles if you have at least two Pokemon, or if other options have limited your party size to one Pokemon."
      * *We have to let the player enter Double Battles if they cap their party size to 1, so that Tate & Liza isn't a softlock.*
    * Permissive: "You can always enter Double Battles, even if you only have one Pokemon on hand."
    * Forced: "All battles are Double Battles, except for specialized battles at the Battle Tower and Battle Frontier. You can enter Double Battles even if you only have one Pokemon on hand."
      * *Requires implementing Wild Double Battles, which in turn requires making some adjustments to wild Pokemon generation and Pokemon catching.*
    * Disabled: "All battles are Single Battles, except for specialized battles at the Battle Tower and Battle Frontier."
      * Includes Multi Battles with NPC allies i.e. Steven.
  * Scale secondary status effect chances
    * "Scale the likelihood that damaging moves will also apply a status effect to their target."
* Daycare and eggs
  * Scale egg hatch time
* Events
  * Rebattling legendary encounters (Allow/Disallow)
    * Set fallback locations for Lati@s? Or just make it so it never stops roaming until caught?
* Infinite Rare Candies from game start, OR an item that lets the player pick a level to advance a Pokemon to.
* Infinite stat vitamins from game start, OR an item that lets the player pick a number of EVs to apply/remove to a Pokemon.
* Nuzlocke
  * Slain Pokemon behavior (Graveyard/Delete)
    * Graveyard: "Boxes in the PC will be repurposed as Graveyards. When one of your Pokemon faints, it will be transferred to a Graveyard. As more Pokemon faint, more boxes will become Graveyards; when no further space remains, fainted Pokemon will be deleted."
    * Delete: "When one of your Pokemon faints, it will be deleted. If you lose a battle, a Pokemon will be automatically withdrawn from the PC to prevent your party from being empty."
  * Encounter Limit (per area)
    * Enable/Disable
    * Applies to... (First Wild Battle/First Obtained)
      * First Wild Battle: "The first wild battle you end up in will count toward the encounter limit: catch the first Wild Pokemon you see in an area, or catch nothing at all."
        * *For obvious reasons, the effect of this option needs to be delayed until the player obtains any Poke Ball. Ergo we need some way to track whether the player has ever had a Poke Ball.*
      * First Obtained: "The first Pokemon you catch or obtain in an area will count toward the encounter limit."
    * Gift Pokemon behavior (Free/Limits/Disable All Gifts)
      * Free: "Gift Pokemon don't count toward encounter limits."
      * Limits: "Gift Pokemon count as your encounter for the area. If you already caught a Pokemon from that area, you cannot receive the gift."
      * Disable All Gifts: "You will not be allowed to receive Gift Pokemon."
    * Shiny Exception (None/Replace Prior/Always Allow)
      * None: "Shiny Pokemon are not an exception to the encounter limit."
      * Replace Prior: "If you encounter a Shiny Pokemon in an area where you've already obtained a Pokemon, you may catch the shiny and replace the other Pokemon. The replaced Pokemon will be treated as if it has fainted."
      * Always Allow: "Shiny Pokemon will not count toward the encounter limit."
  * Dupes Clause (Enable/Disable)
    * "If enabled, you cannot catch more than one of a given Pokemon species."
    * Dupes Clause Shiny Exception (None/Replace Prior/Always Allow)
      * Replace Prior: "If you encounter a Shiny Pokemon after having already obtained a Pokemon of the same species, you may catch the shiny and replace the other Pokemon. The replaced Pokemon will be treated as if it has fainted."
  * Deactivation (Never/After Champion/After Rayquaza/After All Legendaries)
* Pokemon types
  * Control player types
    * Enabled (Yes/No)
    * Limit legal species by type
      * "Limit the species that the player can use in battle, by type. If the player doesn't have any legal Pokemon on hand, they instantly lose any battle that they wind up in."
      * *One checkbox per type.*
    * Override Pokemon types
      * "Override the types of all Pokemon fielded by the player."
      * Type 1
      * Type 2
        * None
        * Any
    * Limit legal moves by type
      * "Limit the moves that the player's Pokemon are allowed to learn."
      * *One checkbox per type.*
      * *When the player obtains a Pokemon, delete any non-matching moves, and disallow learning any illegal moves (manually or via the Daycare). Arrange it so that if none of the Pokemon's moves are legal, it re-learns any forgotten moves that may be legal; if there are no such moves, the Pokemon uses Struggle until it learns a legal move.*
    * Override move types
      * "Override the types of all moves used by the player."
      * *One option per type: map from a source type to a desired type, with a "Use Default" option. Have a "Default" option up top, set to "Unchanged" initially, so that if the player wants to e.g. force all moves to Fire, they don't need to change a dozen and a half separate enums.*
  * Control enemy types
    * Enabled (Yes/No)
    * Override Pokemon types
    * Limit legal moves by type
    * Override move types
* Replace the three starters.
  * Dynamically alter the rival's starter to match the replacements.
* Shop prices
  * Scale buy prices
  * Scale sell prices
* Single-species run
  * "Limit the player to using Pokemon of a single species. This setting overrides any options pertaining to starter Pokemon."
  * Enabled (Yes/No)
  * Player species
  * Allow whole evolution family (Yes/No)
    * "Control whether the player is allowed to evolve members of the target species, and whether the player is allowed to obtain eggs that hatch into pre-evolutions of the target species."
    * *If set to No, then the Daycare will never produce eggs that hatch even into a pre-evo.*
  * Obtaining Pokemon (Limit Catches/Polymorph/Give At Milestones)
    * Limit All Obtained: "The player can only catch members of the target species."
      * *The Daycare will never produce eggs that hatch into any other species.*
    * Polymorph: "All Pokemon caught or obtained by the player transform into the target species."
    * Give At Milestones: "The player is given a member of the target species after each Gym Badge, up to 6."
* Trade Evolutions (Unchanged/Stones/Level Ups)
  * "Replace trade evolutions with other evolution methods."
  * Stones: "All Pokemon that ordinarily evolve via trading will instead be able to evolve using evolution stones."
  * Level Ups: "All Pokemon that ordinarily evolve via trading will instead be able to evolve by leveling up."
* Wild encounters
  * Scale encounter rates
    * *Implementation: see `WildEncounterCheck` in `wild_encounter.c`.*
    * Walking
    * Running
    * Biking
    * Biking only scales while moving (Enabled/Disabled) (default: Disabled)
      * "Control whether the biking multiplier applies cumulatively to things that happen while you're on a bike, such as Rock Smash."
      * *Battle odds are reduced while on a bike in vanilla, since otherwise, travel would be unbearable. However, the vanilla game applies the bike scaling in the "compute wild odds" function, without caring why the function was called. This means that if you're completely stationary on a bike and use Rock Smash, you still get a reduction in battle odds.*
    * Fishing
    * Surfing
    * Rock Smash
    * White Flute multiplier (150%)
    * Black Flute multiplier (50%)
    * Cleanse Tag multiplier (2/3)
  * *Move Pokemon-catching options into here.*

### Custom game option behavior
* Allow the player to change options mid-playthrough
  * Save the fact that they've made these changes
  * Only allow changing an option when doing so would have an effect
    * Changing "Start game with running shoes" should be disabled after starting a playthrough
    * Changing the starter options should be disabled after obtaining a starter
    * NOTE: The CGO menu must know where it was opened from (e.g. New Game/title; in-game Options), in order to enforce this only when it should.

## Engine

### Battles
* Pokemon Custom Emerald allows the player to voluntarily forfeit trainer battles. Don't allow the player to forfeit story-critical trainer battles (i.e. any battle against a villain).
  * I've added a `CurrentBattleAllowsForfeiting()` function in `battles/battle_allows_forfeiting.c`. Whatever flag I add to indicate these battles can be checked for in there.
* Battle ambient weather improvements
  * Some weathers fade the color palettes, e.g. rain darkening the battle scene. Make it so that when you enter the Bag or Party menus, these fades are maintained. Currently, the weather fade reverts simultaneously while the screen fades to black, despite us not taking any action to undo the fades ourselves.
* Ensure that custom game options don't break Recorded Battles. We should probably find a way to store the battle-related CGOs that are in effect during a Recorded Battle, with the data for said recorded battle.
* Fairy type
* Physical/special split

### Cheat/debug menus
* Field Debug Menu
  * Use any field move
    * Implement Secret Power.
* Battles
  * ...
* Utility
  * Test audio
    * Cries
    * Music
    * Sounds
  * Test Pokemon sprites (icon, front, back, palettes)
  * Test trainer sprites
  * Test battle animations
  * Set up test battle
    * Battle type (single, double, multi; trainer, wild)
    * Hand-edit player and enemy teams
    * Hand-edit ally teammates (if a Multi Battle)
* Detect vanilla GameShark/ActionReplay codes
  * Warn the user that these are unsafe for ROM hacks
  * Instruct them on how to reach the hidden cheat menus
  * Reassure them that we won't keep track of what they've tried or not tried to do

### Items
* Investigate separating item IDs into per-bag-pocket ID ranges. This is something we'll want a compiler plug-in to do, but it'd allow bitpacking the player's inventory (which is stored per-pocket) within savedata even more tightly.
  * Depending on how far we can take this, it may also enable expanding the list of Poke Balls without needing all ball item IDs to be consecutive. This would make it considerably easier to add Poke Balls. However, that'll depend on things like redoing how the "mon data" for Poke Balls is stored, among other things.
* Using a Rare Candy on a Lv.100 Pokemon that evolves via level-ups should consume the item and begin an evolution cutscene, unless the Pokemon is holding an Everstone.

### Overworld
* Support for dynamic overworld palettes
* Modify the tile IDs used by overworld dialog boxes, overlapping the ID ranges of mutually-exclusive boxes, so we can use tile IDs closer to the end of VRAM. This will free up more tile IDs for use by overworld tilesets.

### Pokemon forms
* Deoxys (selectable form *a la* Gen IV, instead of fixed per-game-version form *a la* Gen III)
* Regional forms
  * Alolan
  * Galarian
  * Hisuian
  * Paldean
* Non-core-series variations by species
  * Arbok pattern variants ([see Bulbapedia](https://bulbapedia.bulbagarden.net/wiki/Arbok_variants))
  * Magikarp scale patterns and colors, from Magikarp Jump ([see Bulbapedia](https://bulbapedia.bulbagarden.net/wiki/Variant_Pok%C3%A9mon#Magikarp))
  * Can we make newly-hatched Vulpix have a single white tail, per its Pokedex entry? (Perhaps we could gate the sprite out based on level.)
* Core-series form variations
  * Burmy, Gastrodon, etc.

### RTC

Investigate the feasibility of adding real-world date/time tracking to the game.

GBA cartridge RTC chips can store a date and time. Pokemon RSE just has the player enter the current time of day, and treats that as an offset from the RTC-side date and time, without paying any attention to the date and time in any absolute sense. However, other games take other approaches: *Boktai: The Sun Is in Your Hand* has the player enter the date, time, and time zone when they start a playthrough, and it writes these to the RTC and tracks them persistently thereafter.

If we were to allow the player to set the date and time, we could potentially implement real-world seasons and holidays alongside a day/night cycle. We could also store the dates on which Pokemon are caught, the date on which the player starts the game, the date on which the player beats the Elite Four, and other dates that Gen IV onward track. We could let the player change their date, time, and time zone as is possible in the DS games (i.e. disable daily events for 24 hours to prevent cheesing).

Time zones are simple enough: don't bother implementing real-world named zones; just have the player input their offset from UTC. Date and time handling is likely to be more challenging without OS support, e.g. accounting for leap years, leap seconds, February, and whatever else. Allowing the player to change the current date could offer user-side remedies for anything that's too complex or too variable for us to deal with.

One potential wrinkle: emulators support the RTC. How do they manage the date and time stored therein?

### Scripting (overworld)

* Investigate making multiple-choice menus scriptable, such that the script can decide the quantity and text of the selectable options.


### Sprites

Pokemon icon sprites use some cursed trickery to load only a single frame of their animation into VRAM at a time &mdash; necessary because the PC can potentially show up to 36 such sprites at a time, and having each one load both of its animation frames at once would burn too much VRAM.

Game Freak's sprite library has a system for dynamically allocating VRAM for sprite tiles, when loading the sprite. However, the Pokemon icon library uses a bunch of nasty hacks to ensure that only a single frame of the icon loads at a time. These hacks make it dangerous to manage Pokemon icon sprites using the standard sprite-handling functions (which is why there *is* an entire library for Pokemon icons, much of which is dedicated to partially reimplementing sprite functionality with workarounds for what those nasty hacks break).

It'd be nice if we could modify the core sprite library to support keeping only a sprite's current frame of animation in VRAM, and then strip away most of the Pokemon icon code and fall back on standard sprite handling. See documentation on `pokemon_icon.h` for information.


## Features

* Allow the player to rename Pokemon from the PC menu.
* Ball Capsules
  * Gen IV only stored up to 12 designs, and let you assign these to a Pokemon as desired. It wasn't "one design for every Pokemon," so it might actually be viable to implement.
* Overworld follower Pokemon
* The ability to independently adjust the volume of sound effects versus music.


## Pacing

### Battles

* Concurrent animations and text for stat changes.
  * Separate battle script commands trigger the animation and the text, and both work by broadcasting messages to the battle controllers and waiting for a response. Changing this flow risks breaking vanilla link compatibility, and would require a unified message for battle controllers (i.e. "message, animation, and optional minimum duration").
* Health bar changes should animate in proportion with the damage done. When a Lv. 100 Mewtwo annihilates some bug in Viridian Forest, the health bar should drain rapidly both to emphasize the sheer power of the hit and to avoid keeping players waiting as they swat trivial foes out of their way.
  
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
  * Investigate adding format codes (e.g. for species names, move names, common strings like POKeMON and TRAINERS, etc.)
* Investigate support for more text formatting
  * Italics/oblique
    * I think italics would ideally be a `TextPrinter` state bool, rather than a whole separate font. Besides, fonts in this game don't support kerning or any other form of overlapping characters, so a pre-designed italic font wouldn't really *work* here.
    * The vanilla game blits a decompressed font glyph onto the `TextPrinter`'s window tileset using an inline function, `GLYPH_COPY`, in `text.c`. Theoretically, we could add support for programmatically generated oblique fonts by giving that function an `isItalic` parameter, and then having it just blit characters one pixel further to the right once it's past a certain Y-coordinate. Of course, we'd have to be careful to compute string widths and line wrapping properly, by preemptively adding 1px to the computed widths of any lines.

## UI

### Overworld

* Hotkey menu: Allow hotkeying multiple items, with a menu letting you pick which to use.
  * Also requires UI changes to the bag in order to manage hotkeys.
* Topbar
  * Implement as a feature, but disable or make optional for QoL.
  * This could show the current the health of the player's party lead, and the player's Run toggle state. For hacks with day/night cycles, it could show the current in-game time. We'd want the classic 6x6px font, so the topbar can be just a single tile high.

#### Dialogue

* A UI widget to display the name of the speaking NPC.
* Unit portraits

### Battles

* Button to re-show the enemy trainer's party layout (i.e. the Poke Balls shown at battle start and when they're switching out)
* Button to show your moves' descriptions, typing, physical/special, etc..

### Party menu

* Mini-sprites for battle-only forms (e.g. Castform transformations), as in Gen V

### PC

* Releasing a Pokemon should return its held item to the bag.
  * If there's insufficient bag space, then show a confirmation prompt warning that the held item will e lost.
    * Better yet: redirect to player's PC if possible; warn of loss if not.
* Wallpapers
  * The menu for choosing a wallpaper should use two columns, not one, and should support horizontal scrolling.
  * Add more wallpapers! A Spinda-themed design would be nice, and would fit with the custom title screen.
* The context menu for Pokemon should allow assigning/taking their held item.

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

### Trainer Card

* Aim for a more FR/LG-like design.
  * Lighten the area behind the ID number, for legibility.
  * Reduce space above the player's name, to better vertically align things and to potentially make room for another text row.
  * Consider moving the trainer ID to the back of the card, and then, on the front of the card, having the player's name where the ID used to be, without a prefix. This would allow another row of text on the front of the card (not sure what to do with that yet, though).
    * If we get RTC date/time working, we should show the start date of the player's adventure here.
  * Show the Pokemon team icons along the bottom of the back of the card.
* Consider allowing the player to customize the logo behind the player (e.g. full Poke Ball as in RSE; partial Poke Ball as in FR/LG; star; circle; crescent moon).
  * Actually, how far can we take this? Could Halo 3-style Emblems be possible?

## Vanilla bug fixes

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