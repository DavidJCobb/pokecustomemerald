
# Bag menu

The code for the bag menu lives in `item_menu.h`/`.c` and `item_menu_icons.h`/`.c`. To understand it, it's first worth going over the different contexts in which the bag can be opened, and the different interactions you can engage in within the bag.

## The basics of the bag menu

### Interactions
* Choose an item
* Interact with an item
  * Use
  * Toss
  * Check berry tag
* Navigate between bag pockets
* Reorder items within a bag pocket

### Contexts
* View the player's bag (via the overworld pause menu)
* Choose a berry to plant
* Choose a berry to blend/cruch
* Deposit items in the player's PC
* Use an item in battle
* Use an item during the Wally tutorial battle
* Sell items
* Give or show an item to an NPC
  * Give
    * Lilycove Lady (Favor)
  * Show
    * Battle Frontier apprentice
    * Lilycove Lady (Quiz)

### Item sources
* The player's inventory
* The player's Battle Pyramid bag
* Wally's inventory

### Graphics
* Background layers
  * **Layer 0: Content.** The text content of the bag menu is displayed here. This includes the current pocket's title, the listing of items, and the description of the currently focused item.
  * **Layer 1: Modals.** All pop-up windows are displayed here. This includes context menus shown when activating an item, and message windows displayed by an item (e.g. "DAD's advice..." when trying to use a non-usable item).
  * **Layer 2: Chrome.** All borders and background images for layer 0.
* Background windows
  * Bag pocket title
  * Item listing
  * Item description
    * When a TM/HM's stats are being displayed, the description window is not used, and the menu directly pastes tiles into the tilemap for this part of layer 0.
  * Message window
  * Context menus
* Sprites
  * Bag pocket left/right scroll indicators
  * Bag pocket Poke Ball icon (only while switching pockets)
  * Item listing up/down scroll indicators
  * An image of the player's physical bag, with a physical pocket open
  * Item icon, for currently focused item

### Interaction availability by context
Certain contexts only allow certain interactions.

* Changing bag pockets is disabled when selecting a berry, whether to plant or to blend/crush.
* Items cannot be reordered in any other context than when accessing the inventory via the pause menu or during battle, and the TM/HM and Berry pockets in specific can't ever be reordered. This is enforced by `CanSwapItems`.

## Menu behaviors

### Switching to a bag pocket
This has multiple visual transitions, and as such is a latent operation:

* When you switch pockets to the right, the name of the previous pocket is scrolled out to the left, and the name of the next pocket is scrolled in from the right.
* Next to the pocket title is a small Poke Ball icon. This icon actually rotates 180 degrees during the bag-pocket switch. There is a static version of the icon baked into the background chrome layer, and during the switch, a visually identical sprite is spawned overtop it and animated to rotate. The sprite is destroyed when the rotation finishes.
* The chrome for the item listing consists of a lightly-shaded area that the listing text is drawn on. When switching pockets, the lightly-shaded area is reset to zero height, and then expands downward to fill the listing area, before text is then painted. This is essentially a downward-wipe transition, to create a visual "reset" when going from one item listing to another.

The functions involved are:

| Function | Latent | Purpose |
| :- | :- | :- |
| `GetSwitchBagPocketDirection` | No | Input handler. Checks whether the button mappings for switching bag pockets have been activated, and if switching pockets is currently allowed. |
| `SwitchBagPocket` | Sorta | Initiates the process of switching the bag pocket. Switches the task handler to `Task_SwitchBagPocket`, queueing the current task function to continue afterward. |
| `Task_SwitchBagPocket` | Yes | Handles switch-pocket inputs, so that you can rapidly switch across multiple pockets without having to wait for each animation to continue. Handles the "downward wipe" transition and repainting of the pocket name. When the pocket switch timer fully elapses, finalizes the switch, including by calling `ChangeBagPocketId`, before moving to the task follow-up function. |
| `DrawItemListBgRow` | No | Repaints a single background row for the item listing, as part of the "downward wipe" transition. |
| `DrawPocketIndicatorSquare` | No | Repaints the carousel dots shown below the pocket title. |
| `ChangeBagPocketId` | No | Given a pointer to the current bag pocket ID, and a "delta" value (e.g. +1, -1), applies the delta, with wraparound, as appropriate. Note that if the delta is any value other than +/- 1, wraparound won't apply properly, and the bag pocket ID may overflow or underflow. |
