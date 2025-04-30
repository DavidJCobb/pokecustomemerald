
# Edge-cases to design around

When designing your own story, quests, and regions, it is helpful to be aware of edge-cases that have appeared in official games and other ROM hacks.

## Pokemon catching

* If the story requires capture of a legendary to advance, then ensure that you handle the case of the player having insufficient space in both their party and their PC.
  * X, Y, ORAS, SwSh, and LA lock the last PC box from view and use until you catch the necessary legendary, to guarantee that storage is available for it.
  * ORAS doesn't allow you to progress to the Latios/Latias gift until you have room in the PC (even if there's room in your party!).
  * Sun and Moon allow you to re-battle the necessary legendaries endlessly until you catch them, but you earn no EXP for doing so, and the story will not progress until they're caught.
  * SwSh doesn't allow the legendary battle to begin until space is made available.

## Map design

* Ensure that trainers' movement paths can't trap the player. Verify all possible lines of sight for both stationary and moving trainers.
* Ensure that trainers' movement paths and lines of sight cannot cause them to walk over solid obstacles.
* Ensure that overworld teleports can't be used to bypass story triggers.
  * Fly
  * Teleport
  * Fainting from overworld poison damage
* Ensure that "mark and recall"-style effects can't place the player on top of NPCs or objects that appear at the recall point while the player is away.
  * Example: In Diamond and Pearl, you can wait until late night on Friday, stand in a particular spot where Barry appears on Saturdays, enter the Underground, and pop up from the Underground once the clock ticks over to Saturday.

### Date and time

* If an NPC should only appear during a certain time frame (e.g. a weekday), ensure that players can't break their behavior by idling on the overworld for their appearance or disappearance time.
  * Example: in Diamond and Pearl, Barry appears in a particular place on weekends. The player can stand there, let the clock tick over to a weekday, and interact with him as if it were a weekend.