
# `pokemon_icon.h`

## Notes

* ~~Sprites created via `CreateMonIconSprite` cannot be safely destroyed via `DestroySprite`. This is because `CreateMonIconSprite` stores the sprite's `images` list in a local buffer (lost when the function exits), but also has the sprite dynamically allocate its tiles. Dynamically-allocated tiles cannot be freed if the `images` pointer becomes invalid.~~ Resolved in this fork via changes to the core sprite library.