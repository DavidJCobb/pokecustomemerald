#include "lu/naming_screen/core.h"
#include "lu/widgets/keybind_strip.h"

#include "global.h"

#include "bg.h"
#include "palette.h"
#include "sprite.h"
#include "task.h"
#include "text_window.h"
#include "window.h"

#define KEYBOARD_PAGE_COUNT 3
#define KEYBOARD_PAGE_ROWS  4
#define KEYBOARD_PAGE_COLS  8

enum KeyPage {
   KEYPAGE_LETTERS_LOWER,
   KEYPAGE_LETTERS_UPPER,
   KEYPAGE_SYMBOLS,
};

struct KeyboardPage {
   const u8 characters[KEYBOARD_PAGE_ROWS][KEYBOARD_PAGE_COLS];
   u8       column_count;
};

static const struct KeyboardPage keyboard_pages[] = {
   [KEYPAGE_LETTERS_LOWER] = {
      .column_count = KEYBOARD_PAGE_COLS,
      .characters   = {
         __("abcdef ."),
         __("ghijkl ,"),
         __("mnopqrs "),
         __("tuvwxyz "),
      },
   },
   [KEYPAGE_LETTERS_UPPER] = {
      .column_count = KEYBOARD_PAGE_COLS,
      .characters   = {
         __("ABCDEF ."),
         __("GHIJKL ,"),
         __("MNOPQRS "),
         __("TUVWXYZ "),
      },
   },
   [KEYPAGE_SYMBOLS] = {
      .column_count = 6,
      .characters   = {
         __("01234   "),
         __("56789   "),
         __("!?♂♀/-  "),
         __("…“”‘'   "),
      },
   },
};