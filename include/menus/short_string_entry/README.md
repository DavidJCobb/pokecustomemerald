
# To-do

Earlier in development, I wanted to add the ability to use accented characters when naming the player and when naming Pokemon. It ended up being easiest to just recreate the "naming screen" from scratch, so I did. I didn't get around to wholly replacing the vanilla naming screen, but the new naming screen opens when naming a Pokemon via the party menu, and when using a debug command to rename the player.

However:

* I've come to really dislike the visual design I did for it (just something I hastily concepted out so I'd have *something* to test with).

* I'm trying to refactor all of the custom menus I've been building. I want a top-level `menus` folder, and one subfolder per menu, with menus' individual subsystems or (non-graphical) resources stored inside.

So I've been trying to...

* Create a "short string entry menu" with the refactored folder structure, dividing menu components up into more granular files so that the menu can be tackled one piece at a time.

* Replace the new "naming screen" with it.

* Replace the vanilla "naming screen" with it.



## Remaining tasks

* Finish implementing menu cursors within the short string entry menu.

  * The "custom keyboard" widget has a built-in cursor. We can't influence its appearance or palette from outside, which is perhaps an issue.
  
    * Maybe we should just try red as the cursor color elsewhere and see how it looks.
  
  * Charset button cursor needs a little spike on the top that the sparks come from.

* Improve the rays-of-light effect in the menu backdrop, or replace it with something else.

  * It occurs to me that we could have particle sprites drifting softly in the background, behind the white-shaded background layer.

* Add a way to control which charsets are enabled. Walda's Password should only accept normal Latin symbols. (Low-priority.)

  * This would require changing which charsets we give to the custom keyboard, and writing logic to lay out the charset buttons (centering, spacing, VUI context grid, etc.) based on which ones ought to be visible.

* When the new(est) menu is confirmed to have feature parity with lu-naming-screen, make the "rename via party menu" feature use it, and wholly remove lu-naming-screen.

* Finish replacing the vanilla naming screen with the short-string-entry menu.

  * When you rename a freshly caught Pokemon that's bound for the PC, you're shown a message box telling you what box it'll end up in. This box displays while still on the vanilla naming screen. We need to update our short string entry menu to support this, i.e. to have an "on before exit" callback that takes over the menu task and is capable of showing message boxes. This also means we should avoid using, in VRAM, the same palette indices and tile IDs as the vanilla message boxes and dialog frames.
