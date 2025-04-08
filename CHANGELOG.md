
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
      * No backfield healing at all
      * No revives
    * Wager options (money gain/loss)
  * Catching Pokemon
    * Catch rate flat increase
    * Catch rate scaling
    * Catch EXP toggle
  * Overworld
    * Start game with running shoes
    * Run indoors
    * Bike indoors
    * Poison damage and interval

* Coalesced all item pick-up text. The vanilla game uses two textboxes: one to tell you what you picked up, and another to tell you what bag pocket you put it in. We combine these into one textbox.

* Sped up the game's startup sequence: it fades in from white more quickly, the copyright screen is displayed for half as long, and the transition from the copyright screen to the intro animation is faster.

* Substantially reduced the delay before Professor Birch appears in the New Game intro.

* Removed the grey flashes at the start of all battle transitions, to waste less time when going from the overworld to a battle.

* Increased the speed of the battle start slide-in animation by 50%.

* Slightly increased the speed of the battle start letterbox animation. This doesn't allow you to actually start engaging with the battle faster (the slide-in animation is the trigger), but it makes the intro feel slightly more responsive.

* The vanilla game always plays stat-change animations, even when battle animations are disabled, because it's awkward to have a visual affordance (blinking sprite) when damage is taken but not when stats are changed. Custom Emerald also plays stat-change animations, but when battle animations are disabled, we use faster timings and higher opacity for stat-change animations.

* Minor, as-yet-untested bugfixes.

  * Activating a PokeCenter with an empty party should no longer lock you in a 256-ball loop.

  * The party menu underflow that allows "glitzer popping" should no longer be possible.