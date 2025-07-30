
# Yes/No modal

Reusable code to spawn a "yes/no" choice modal dialog.

This has a few differences from the vanilla functions (`DoYesNoFuncWithChoice` and `CreateYesNoMenuWithCallbacks` in `menu_helpers.h`):

* The vanilla menu helper relies on `sMenu`, a static struct in `menu.h`. This one struct has to have parameters for all possible menus that it can be used to display, and is statically allocated. By contrast, we spawn a specialized data structure for our modal's state, and this struct is heap-allocated and freed as appropriate.
* The vanilla functions require the caller to supply a full `WindowTemplate`, whereas our functions take only those parameters that might vary from caller to caller (i.e. we don't require you to specify the width and height).
* The vanilla menu helper code is procedural rather than OO: the various built-in menus (including the yes/no modal) are decomposed along lines of behavior. This means you're kind of navigating all over the place to track down the different pieces of functionality.

## Usage

Create an instance of either `struct LuModalYesNoParams` or `struct LuModalYesNoSplitParams`, and pass its pointer to `LuModalYesNo`. (The use case for the "split params" is when you have multiple possible yes/no boxes in a single menu, which need different callbacks but which have identical graphics parameters. You can define the unchanging parameter sets as `static const` structs to which the split-params object points.)

There are two kinds of callbacks the menu supports: a single callback, which takes a boolean parameter; or dual callbacks, one for yes and one for no, which take no arguments.

There are three kinds of task management that are supported:
* Have the modal spawn and manage its own task.
* Have the modal take over an existing task. The task function (at the time of the modal spawning) will be restored at modal exit, before the callback is invoked.
* Have the modal take over an existing task. The task will be set to a no-op function at modal exit, before the callback is invoked.