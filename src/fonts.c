#include "global.h"

ALIGNED(4) const u16 gFontSmallNarrowLatinGlyphs[] = INCBIN_U16("graphics/fonts/small_narrow.latfont");
ALIGNED(4) const u8 gFontSmallNarrowLatinGlyphWidths[] = {
    3,  5,  5,  5,  5,  5,  5,  5,  5,  4,  3,  4,  4,  5,  5,  5,
    5,  5,  5,  5,  5,  5,  5,  5,  3,  4,  5,  5,  5,  5,  4,  3,
    4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  8,  5,  6,  3,
    3,  3,  3,  3,  8,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    5,  5,  3,  8,  8,  8,  8,  8,  8,  8,  4,  5,  4,  4,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  5,  3,  3,  3,  3,  3,  3,  4,
    3,  3,  3,  3,  3,  3,  3,  5,  3,  8,  8,  8,  8,  1,  2,  3,
    4,  5,  6,  7,  5,  5,  5,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    7,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  5,  3,  5,  5,
    5,  5,  5,  3,  3,  5,  5,  5,  3,  5,  5,  5,  5,  5,  5,  5,
    5,  5,  5,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  5,
    5,  5,  5,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  4,  5,
    4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  7,
    3,  5,  5,  5,  5,  5,  5,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  3,
};

ALIGNED(4) const u16 gFontSmallLatinGlyphs[] = INCBIN_U16("graphics/fonts/small.latfont");
ALIGNED(4) const u8 gFontSmallLatinGlyphWidths[] = {
    3,  5,  5,  5,  5,  5,  5,  5,  5,  4,  3,  4,  4,  5,  5,  5,
    5,  5,  5,  5,  5,  5,  5,  5,  3,  5,  5,  5,  5,  5,  4,  3,
    4,  4,  5,  5,  5,  6,  5,  5,  5,  5,  5,  5,  8,  7,  8,  3,
    3,  3,  3,  3,  8,  8,  7,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    5,  5,  5,  8,  8,  8,  8,  8,  8,  8,  4,  7,  5,  5,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  5,  3,  3,  3,  3,  3,  3,  4,
    3,  3,  3,  3,  3,  3,  3,  5,  3,  8,  8,  8,  8,  1,  2,  3,
    4,  5,  6,  7,  5,  7,  7,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  5,  5,  5,  5,
    5,  5,  5,  5,  5,  5,  5,  8,  5,  8,  5,  5,  5,  5,  5,  5,
    5,  5,  5,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  5,
    5,  5,  5,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  5,  5,
    5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  8,
    7,  5,  5,  5,  5,  5,  5,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  3,
};

ALIGNED(4) const u16 gFontNarrowLatinGlyphs[] = INCBIN_U16("graphics/fonts/narrow.latfont");
ALIGNED(4) const u8 gFontNarrowLatinGlyphWidths[] = {
    3,  5,  5,  5,  5,  5,  5,  5,  5,  4,  3,  4,  4,  5,  5,  5,
    8,  5,  5,  5,  5,  6,  5,  5,  3,  5,  5,  5,  5,  5,  4,  3,
    4,  4,  5,  5,  5,  8,  5,  5,  5,  5,  5,  6,  9,  6,  6,  3,
    3,  3,  3,  3,  8,  8,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    5,  5,  4,  8,  8,  8,  7,  8,  8,  4,  4,  6,  4,  4,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  5,  3,  3,  3,  3,  3,  3,  4,
    3,  3,  3,  3,  3,  3,  3,  5,  3,  7,  7,  7,  7,  1,  2,  3,
    4,  5,  6,  7,  5,  6,  6,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  5,  3,  5,  3,
    5,  5,  5,  3,  3,  5,  5,  6,  3,  6,  6,  5,  5,  5,  5,  5,
    5,  5,  5,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
    5,  5,  5,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  5,  5,
    4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  8,
    3,  5,  5,  5,  5,  5,  5,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    10, 10, 10, 10,  8,  8, 10,  8, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  3,
};

ALIGNED(4) const u16 gFontShortLatinGlyphs[] = INCBIN_U16("graphics/fonts/short.latfont");
ALIGNED(4) const u8 gFontShortLatinGlyphWidths[] = {
    3,  6,  6,  6,  6,  6,  6,  6,  6,  6,  3,  6,  6,  6,  6,  6,
    8,  6,  6,  6,  6,  6,  6,  6,  3,  6,  6,  6,  6,  6,  6,  3,
    6,  6,  6,  6,  6,  8,  6,  6,  6,  6,  6,  6,  9,  8,  8,  3,
    3,  3,  3,  3, 10,  8,  5,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    6,  6,  6,  8,  8,  8,  8,  8,  8,  4,  6,  8,  5,  5,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  6,  3,  3,  3,  3,  3,  3,  6,
    3,  3,  3,  3,  3,  3,  3,  6,  3, 12, 12, 12, 12,  1,  2,  3,
    4,  5,  6,  7,  8,  8,  8,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  6,  5,
    6,  6,  6,  3,  3,  6,  6,  8,  5,  9,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  6,  6,  4,  6,  5,
    5,  6,  5,  6,  6,  6,  5,  5,  5,  6,  6,  6,  6,  6,  6,  8,
    5,  6,  6,  6,  6,  6,  6,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    12, 12, 12, 12,  8, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  3,
};

ALIGNED(4) const u16 gFontNormalLatinGlyphs[] = INCBIN_U16("graphics/fonts/normal.latfont");
ALIGNED(4) const u8 gFontNormalLatinGlyphWidths[] = {
    3,  6,  6,  6,  6,  6,  6,  6,  6,  6,  3,  6,  6,  6,  6,  6,
    8,  6,  6,  6,  6,  6,  6,  6,  3,  6,  6,  6,  6,  6,  6,  3,
    6,  6,  6,  6,  6,  8,  6,  6,  6,  6,  6,  6,  9,  7,  6,  3,
    3,  3,  3,  3, 10,  8,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    6,  6,  4,  8,  8,  8,  7,  8,  8,  4,  6,  6,  4,  4,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  6,  3,  3,  3,  3,  3,  3,  6,
    3,  3,  3,  3,  3,  3,  3,  6,  3,  7,  7,  7,  7,  1,  2,  3,
    4,  5,  6,  7,  6,  6,  6,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  4,  6,  3,  6,  3,
    6,  6,  6,  3,  3,  6,  6,  6,  3,  7,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  4,  5,  6,
    4,  6,  6,  6,  6,  6,  5,  6,  6,  6,  6,  6,  6,  6,  6,  8,
    3,  6,  6,  6,  6,  6,  6,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    10, 10, 10, 10,  8, 10, 10,  8, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  3,
};

ALIGNED(4) const u16 gFontSmallJapaneseGlyphs[] = INCBIN_U16("graphics/fonts/small.hwjpnfont");
ALIGNED(4) const u16 gFontNormalJapaneseGlyphs[] = INCBIN_U16("graphics/fonts/normal.hwjpnfont");

ALIGNED(4) const u16 gFontFRLGMaleJapaneseGlyphs[] = INCBIN_U16("graphics/fonts/frlg_male.fwjpnfont");
ALIGNED(4) const u8 gFontFRLGMaleJapaneseGlyphWidths[] = {
    0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    9,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,  9,
    9,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    9,  8,  7,  8,  8,  8,  8,  8,  8,  8,  8,  5,  9, 10, 10, 10,
    8, 10, 10, 10, 10,  8,  8,  8, 10, 10,  8,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  6,  6,  2,  4,  6,
    3,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    5,  6,  6,  6,  6,  6,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
};

ALIGNED(4) const u16 gFontFRLGFemaleJapaneseGlyphs[] = INCBIN_U16("graphics/fonts/frlg_female.fwjpnfont");
ALIGNED(4) const u8 gFontFRLGFemaleJapaneseGlyphWidths[] = {
    0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,  9,
    9,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,  9,
    9,  9,  9,  9,  9,  9,  8, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    9,  8,  7,  8,  8,  8,  8,  8,  8,  8,  8,  5,  9, 10, 10, 10,
    8, 10, 10, 10, 10,  8,  8,  8, 10, 10,  8,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  6,  6,  2,  4,  6,
    3,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    5,  6,  6,  6,  6,  6,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
};

ALIGNED(4) const u16 gFontShortJapaneseGlyphs[] = INCBIN_U16("graphics/fonts/short.fwjpnfont");
ALIGNED(4) const u8 gFontShortJapaneseGlyphWidths[] = {
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,  9,
    9,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,  9,
    9,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10,  8,  7,  8,  8,  8,  8,  8,  8,  8,  8,  5,  9, 10, 10, 10,
    8, 10, 10, 10, 10,  8,  8,  8, 10, 10,  8,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  6,  6,  2,  4,  6,
    3,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    5,  6,  6,  6,  6,  6,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
};

// Extended
ALIGNED(4) const u16 gFontBoldLatinGlyphs[] = INCBIN_U16("graphics/fonts/bold.latfont");
ALIGNED(4) const u8 gFontBoldLatinGlyphWidths[] = {
    3,  7,  7,  7,  7,  7,  7,  7,  7,  7,  3,  7,  7,  7,  7,  7,
    8,  7,  7,  7,  7,  7,  7,  7,  3,  7,  7,  7,  7,  7,  7,  3,
    7,  5,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7, 11,  8,  7,  3,
    3,  3,  3,  3, 11,  8,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    7,  7,  5,  8,  8,  8,  7,  8,  8,  4,  6,  6,  5,  5,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  7,  3,  3,  3,  3,  3,  3,  5,
    3,  3,  3,  3,  3,  3,  3,  6,  3,  7,  7,  7,  7,  1,  2,  3,
    4,  5,  6,  7,  6,  7,  7,  3,  3,  3,  3,  3,  3,  3,  3,  3, // blank[4], super e, <, >
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, // empty row
   10,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  4,  7,  3,  6,  3, // symbol, 0 - 9, symbols
    9,  6,  6,  3,  3,  7,  7,  7,  3,  8,  7,  7,  7,  7,  7,  7, // symbols, A - E
    7,  7,  7,  7,  7,  7,  7,  8,  7,  7,  7,  7,  7,  7,  7,  7, // F - U
    7,  8,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  3,  6,  7, // V - Z, a - k
    3,  9,  7,  7,  7,  7,  6,  7,  7,  7,  7,  9,  7,  7,  7,  8, // l - z, selection cursor
    3,  7,  7,  7,  7,  7,  7,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    10, 10, 10, 10, 9, 11, 12,  9, 12, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  3,
};
