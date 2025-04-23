
# `sprite.h`

## Changes made in this repo

### Dynamic tile allocation and freeing fix

Game Freak sometimes creates sprites and has them allocate their tiles in OBJ VRAM dynamically. Pokemon icons (created by the functions in `pokemon_icon.h`) are a notable example of this. However, if such a sprite has a stack-allocated `SpriteFrameImage` object (as is the case for those Pokemon icons), then the sprite engine won't be able to properly free the allocated tiles in `DestroySprite`, because the sprite's `images` pointer will be left dangling.

`Sprite::usingSheet` indicates whether a sprite used a fixed spritesheet or dynamically allocated its tiles. In the vanilla game, `Sprite::sheetTileStart` is only used when the sprite has a fixed spritesheet. As such, in this repo, it's been converted into a union; when a sprite dynamically allocates its tiles, the active union member becomes `Sprite::allocTileCount`, which stores the number of tiles allocated. `DestroySprite` has been updated to rely on this value.

### Resource heap allocation

It's common for Game Freak to allocate `SpriteTemplate`s and `SpriteFrameImage`s on the stack, and use these when creating sprites. However, this causes the created `Sprite` to hold dangling pointers, and interferes with cleaning up a sprite's resources:

* If a sprite's tiles are dynamically allocated, they cannot be properly freed if the sprite's `SpriteFrameImage`(s) were on the stack. This breaks some uses of `CreateMonIconSprite` from `pokemon_icon.h`, and is almost certainly why the PC UI code avoids using that function.

This repo co-opts some of the unused `Sprite` flags to indicate whether a sprite owns these sorts of resources. You can call the following resources to have a sprite copy the given resource onto the heap and take ownership of it, such that `ResetSprite` will free the resource from the heap when destroying the sprite.

* `SpriteTakeOwnershipOfImages`
* `SpriteTakeOwnershipOfTemplate`