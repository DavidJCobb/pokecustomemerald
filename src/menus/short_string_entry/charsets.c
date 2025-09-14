#include "menus/short_string_entry/charsets.h"
#include "lu/vui/custom-keyboard.h"

static const u8 sCharsetCharactersUpper[] = __(
   "ABCDEF ."
   "GHIJKL ,"
   "MNOPRQS "
   "TUVWXYZ "
);
static const u8 sCharsetCharactersLower[] = __(
   "abcdef ."
   "ghijkl ,"
   "mnopqrs "
   "tuvwxyz "
);
static const u8 sCharsetCharactersSymbol[] = __(
   "01234 "
   "56789 "
   "!?♂♀/-"
   "…“”‘' "
);
static const u8 sCharsetCharactersAccentUpper[] = __(
   "ÁÂÀÄ  ÇŒ"
   "ÉÊÈË  ßÑ"
   "ÍÎÌÏ    "
   "ÓÔÒÖÚÛÙÜ"
);
static const u8 sCharsetCharactersAccentLower[] = __(
   "áâàä  çœ"
   "éêèë   ñ"
   "íîìï    "
   "óôòöúûùü"
);

const struct VUICustomKeyboardCharset gShortStringEntryMenuCharsets[] = {
   {  // Upper
      .characters = sCharsetCharactersUpper,
      .rows = 4,
      .cols = 8,
      .col_gaps = {
         .count     = 2,
         .positions = { 2, 6 }
      }
   },
   {  // Lower
      .characters = sCharsetCharactersLower,
      .rows = 4,
      .cols = 8,
      .col_gaps = {
         .count     = 2,
         .positions = { 2, 6 }
      }
   },
   {  // Symbol
      .characters = sCharsetCharactersSymbol,
      .rows = 4,
      .cols = 6,
      .col_gaps = {
         .count = 0
      }
   },
   {  // Accent Upper
      .characters = sCharsetCharactersAccentUpper,
      .rows = 4,
      .cols = 8,
      .col_gaps = {
         .count     = 1,
         .positions = { 3 }
      }
   },
   {  // Accent Lower
      .characters = sCharsetCharactersAccentLower,
      .rows = 4,
      .cols = 8,
      .col_gaps = {
         .count     = 1,
         .positions = { 3 }
      }
   },
};