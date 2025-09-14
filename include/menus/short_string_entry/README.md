
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

  * The "custom keyboard" widget has a cursor built-in, though we can't influence its palette, which is perhaps an issue.
  
  * We need to implement cursor sprites for each charset button (setting them to (in)visible as appropriate), and a cursor sprite for the menu buttons ("OK"/"Del").
  
    * Charset buttons all vary in width, which is why it's easiest to just give each its own cursor `Sprite` that we toggle on and off. Menu buttons are a consistent size, so we use just one `Sprite` for it and change that sprite's position as appropriate.
    
  * I have a cursor design that has a simple animation, but I think it'd be easier to implement a static cursor sprite first. Then, we could implement the animation using "particle" sprites -- either statically designed animations that we attach as secondary sprites, or one `Sprite` per particle with affine transforms to shrink them as they drift away from their emitters.

* Update our field debug menu to open the short-string-entry menu instead of the lu-naming-screen menu, so we can test the new(est) menu.

* When the new(est) menu is confirmed to have feature parity with lu-naming-screen, make the "rename via party menu" feature use it, and wholly remove lu-naming-screen.

* Finish replacing the vanilla naming screen with the short-string-entry menu.

  * When you rename a freshly caught Pokemon that's bound for the PC, you're shown a message box telling you what box it'll end up in. This box displays while still on the vanilla naming screen. We need to update our short string entry menu to support this, i.e. to have an "on before exit" callback that takes over the menu task and is capable of showing message boxes. This also means we should avoid using, in VRAM, the same palette indices and tile IDs as the vanilla message boxes and dialog frames.
