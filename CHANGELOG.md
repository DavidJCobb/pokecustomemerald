
# Changelog

## Pre-release

* Added game options, to alter how you interact with the game experience as it is presented to you.

  * Hold/toggle to run

* Added Custom Game Options, to alter the game experience in unusual ways.

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
  * Catching Pokemon
    * Catch rate flat increase
    * Catch rate scaling
    * Catch EXP toggle
  * Overworld
    * Start game with running shoes
    * Run indoors
    * Bike indoors
    * Poison damage and step interval
  * Starter Pokemon
    * Change the three available species
    * Change the initial level of the player's starter
    * Force a gender for the player's starter

* Implemented a custom title screen.

[^no-backfield-inspo]: The option to limit backfield healing was inspired by [a collaboration between Pokémon Challenges and SmallAnt](https://www.youtube.com/watch?v=_3VwGkml-nk) wherein the latter ran a Nuzlocke while the former gained control of all NPC trainers and tried to stop him. Unlike a typical NPC trainer, PChal could sacrifice weaker Pokémon to withdraw his heavy hitters and heal them in safety, and he was ruthless in taking advantage of this, allowing him to effectively wall SmallAnt using Norman's Slaking.

### Pacing

* Coalesced all item pick-up text. The vanilla game uses two textboxes: one to tell you what you picked up, and another to tell you what bag pocket you put it in. We combine these into one textbox.

* Coalesced some text related to interacting with berry trees, to use fewer textboxes and to segment textboxes better.

* Sped up the game's startup sequence: it fades in from white more quickly, the copyright screen is displayed for half as long, and the transition from the copyright screen to the intro animation is faster.

* Substantially reduced the delay before Professor Birch appears in the New Game intro.

* Changed the pacing of the Pokemon Center healing animation. The nurse takes Poke Balls from the player and inserts them into the machine two at a time.

#### Battles

* Removed the grey flashes at the start of all battle transitions, to waste less time when going from the overworld to a battle.

* Increased the speed of the battle start slide-in animation by 50%.

* Slightly increased the speed of the battle start letterbox animation. This doesn't allow you to actually start engaging with the battle faster (the slide-in animation is the trigger), but it makes the intro feel slightly more responsive.

* The vanilla game always plays stat-change animations, even when battle animations are disabled, because it's awkward to have a visual affordance (blinking sprite) for when damage is taken but not for when stats are changed. Custom Emerald also plays stat-change animations, but when battle animations are disabled, we use faster timings and higher opacity for stat-change animations.

* The animation that a Pokemon plays when consuming a held berry is slightly briefer: the Pokemon angles down to munch on the berry twice rather than three times.

### Vanilla bug-fixes and engine improvements

* Slight refactor to the options menu, to support scrolling. This makes it slightly easier to add a few new options, but since each option's current value is still stored in task data, a further refactor will be needed to add terribly many more.

* Added script commands to facilitate displaying text with automatic word-wrapping performed at run-time.

* Minor, as-yet-untested bugfixes.

  * Activating a PokeCenter with an empty party should no longer lock you in a 256-ball loop.

  * The party menu underflow that allows "glitzer popping" should no longer be possible.