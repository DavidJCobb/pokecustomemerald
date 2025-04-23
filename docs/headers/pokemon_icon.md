
# `pokemon_icon.h`

Pokemon icons are sprites, but they break a lot of the rules of sprites.

* Game Freak's sprite library assumes that if you're loading an animated sprite and dynamically allocating its tiles, you're loading *all* of its animation frames at once. Pokemon icons instead only load the current frame.

* Pokemon icons break the above rule by passing (in `CreateMonIconSprite`) a stack-allocated single-frame `SpriteFrameImage` array when creating the sprite. This single frame has a `NULL` pointer for sprite data, and claims to be one 32x32px frame. After the sprite is created, its `SpriteFrameImage` array is swapped out for a persistent array, containing a single 32x32px `SpriteFrameImage` that points to the first frame of the sprite data. This method of creating the sprite ensures that only the tiles needed for a single frame are reserved; but it breaks `DestroySprite` and prevents the sprite library from properly freeing the icon's tiles on its own.

* The Pokemon icon library provides `FreeAndDestroyMonIconSprite` as an alternative to `DestroySprite`. This creates the same stack-allocated data as is used when setting up the sprite, swaps a pointer to that in, and then calls `DestroySprite`.

* Of course, because the sprite doesn't have all of its animation frames in memory, it can't use the same animation system as every other sprite. `UpdateMonIconFrame` is provided as a partial remedy to this, reimplementing a limited portion of the sprite animation code with custom handling for loading the next frame of animation.

  * Recall that after the sprite is created, it's given a single `SpriteFrameImage` pointing to its first frame. However, the sprite's frames are stored contiguously in ROM, so `UpdateMonIconFrame` just does pointer math to find the second frame's data, and triggers a copy on that data.

This fork has made a few changes to the sprite library:

* Sprites that allocate tile VRAM dynamically will now store how many tiles they allocated, and this information is used during teardown. This removes the need to use `FreeAndDestroyMonIconSprite`.

## Notes

* ~~Sprites created via `CreateMonIconSprite` cannot be safely destroyed via `DestroySprite`. This is because `CreateMonIconSprite` stores the sprite's `images` list in a local buffer (lost when the function exits), but also has the sprite dynamically allocate its tiles. Dynamically-allocated tiles cannot be freed if the `images` pointer becomes invalid.~~ Resolved in this fork via changes to the core sprite library.