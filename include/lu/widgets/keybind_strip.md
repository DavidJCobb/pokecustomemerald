
# Keybind strip

A blue bar at the bottom of a window, which displays the available button mappings.

Each keybind strip entry has two fields: a bitmask of buttons mapped to the key; and the text of the entry. Button bits use the same indices as `CHAR_A_BUTTON` and friends in `constants/characters.h` (i.e. to show the L-button, set `buttons |= (1 << CHAR_L_BUTTON)`).

The keybind strip itself maintains a bitmask of which entries to show.