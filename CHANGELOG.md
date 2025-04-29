
# Changelog

## Pre-release

* Added game options, to alter how you interact with the game experience as it is presented to you.

  * Hold/toggle to run

* Added **Custom Game Options**, to alter the game experience in unusual ways.

  * Battle
    * Accuracy scaling
    * Damage scaling
    * EXP scaling
    * Item use by player
      * Vanilla
      * No backfield healing, except revives
      * No backfield healing at all[^no-backfield-inspo]
      * No revives
    * Wager options
      * Scale money gain on victory
      * Alter money loss on defeat (classic/modern)
    * Rematch options
      * Minimum badges before trainers start becoming available
      * Interval at which trainers are re-checked
      * Chance of each trainer becoming available
  * Catching Pokémon
    * Catch rate flat increase
    * Catch rate scaling
    * Catch EXP toggle
  * Daycare and eggs
    * Daycare can teach new moves (enabled/disabled)
    * Scale daycare cost
    * Scale daycare EXP
    * Scale egg laying chance
  * Events
    * Eon Ticket availability
    * Mirage Island
      * Rarity
      * Include boxed Pokémon
  * Overworld
    * Start game with running shoes
    * Run indoors
    * Bike indoors
    * Poison damage, step interval, and optional Gen IV behavior (stop damage at 1HP)
  * Starter Pokémon
    * Change the three available species
    * Change the initial level of the player's starter
    * Force a gender for the player's starter

* Implemented a custom title screen.

* When setting the current time of day, the wall clock now shows the exact hours and minutes, on the left side of the screen.

* When a Repel runs out and the player has more Repels of the same type in their inventory, they'll be told how many they have left, and offered the option to quickly use another.

* The player can now voluntarily **forfeit trainer battles**, even outside of the Battle Frontier and Trainer Hill (where the vanilla game would allow forfeiting). Battles that allow forfeiting will show a "GIVE UP" option instead of "RUN." Clicking this option always pops a confirmation prompt (as it did in the vanilla game).

* Pokémon that aren't gender-unknown are now referred to with gendered pronouns during battle, to make the text feel less impersonal (i.e. "MISDREAVUS hurt herself in her confusion" rather than "MISDREAVUS hurt itself in its confusion").

* You can now rename Pokémon via the party menu.

* When EXP is earned in battles and divided amongst the participants, Lv. 100 Pokemon no longer waste any of the EXP.

* **Battle ambient weather:** In battles, while selecting an action, the animation for the current weather will play in a loop. Additionally, messages like "Rain continues to fall" no longer display, improving pacing.

[^no-backfield-inspo]: The option to limit backfield healing was inspired by [a collaboration between Pokémon Challenges and SmallAnt](https://www.youtube.com/watch?v=_3VwGkml-nk) wherein the latter ran a Nuzlocke while the former gained control of all NPC trainers and tried to stop him. Unlike a typical NPC trainer, PChal could sacrifice weaker Pokémon to withdraw his heavy hitters and heal them in safety, and he was ruthless in taking advantage of this, allowing him to effectively wall SmallAnt using Norman's Slaking.

### Pacing

* Coalesced all item pick-up text. The vanilla game uses two textboxes: one to tell you what you picked up, and another to tell you what bag pocket you put it in. We combine these into one textbox.

* Coalesced some text related to interacting with berry trees, to use fewer textboxes and to segment textboxes better.

* Sped up the game's startup sequence: it fades in from white more quickly, the copyright screen is displayed for half as long, and the transition from the copyright screen to the intro animation is faster.

* Substantially reduced the delay before Professor Birch appears in the New Game intro.

* Changed the pacing of the Pokémon Center healing animation. The nurse takes Poke Balls from the player and inserts them into the machine two at a time.

#### Battles

* Removed the grey flashes at the start of all battle transitions, to waste less time when going from the overworld to a battle.

* Increased the speed of the battle start slide-in animation by 50%.

* Slightly increased the speed of the battle start letterbox animation. This doesn't allow you to actually start engaging with the battle faster (the slide-in animation is the trigger), but it makes the intro feel slightly more responsive.

* The vanilla game always plays stat-change animations, even when battle animations are disabled, because it's awkward to have a visual affordance (blinking sprite) for when damage is taken but not for when stats are changed. Custom Emerald also plays stat-change animations, but when battle animations are disabled, we use faster timings and higher opacity for stat-change animations.

* The animation that a Pokemon plays when consuming a held berry is slightly briefer: the Pokemon angles down to munch on the berry twice rather than three times.

* As mentioned above, "weather continues" messages no longer display, since the "battle ambient weather" feature feels like an acceptable substitute.

### Vanilla bug-fixes and engine improvements

* Slight refactor to the options menu, to support scrolling. This makes it slightly easier to add a few new options, but since each option's current value is still stored in task data, a further refactor will be needed to add terribly many more.

* Added script commands to facilitate displaying text with automatic word-wrapping performed at run-time.

* Minor, as-yet-untested bugfixes.

  * Activating a PokeCenter with an empty party should no longer lock you in a 256-ball loop.

  * The party menu underflow that allows "glitzer popping" should no longer be possible.