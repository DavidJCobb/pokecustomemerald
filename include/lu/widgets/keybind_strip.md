
# Keybind strip

A blue bar at the bottom of a window, which displays the available button mappings.

Each keybind strip entry has three fields: a bitmask of buttons mapped to the key; a boolean indicating whether to show the entry; and the text of the entry. Button bits use the same indices as `CHAR_A_BUTTON` and friends in `constants/characters.h` (i.e. to show the L-button, set `buttons |= (1 << CHAR_L_BUTTON)`).