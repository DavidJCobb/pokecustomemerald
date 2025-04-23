#include "strings/custom_game_options.rematch.h"

extern const u8 gText_lu_CGOptionCategoryName_Rematch[];

extern const u8 gText_lu_CGOptionName_Rematch_MinBadges[];
extern const u8 gText_lu_CGOptionHelp_Rematch_MinBadges[];
extern const u8 gText_lu_CGOptionName_Rematch_Chance[];
extern const u8 gText_lu_CGOptionHelp_Rematch_Chance[];

const u8 gText_lu_CGOptionCategoryName_Rematch[] = _("Rematches");

const u8 gText_lu_CGOptionName_Rematch_MinBadges[] = _("Minimum badge requirement");
const u8 gText_lu_CGOptionHelp_Rematch_MinBadges[] = _("Control the number of badges the player must have before trainers start becoming available for rematches.\n\nDefault: 5.");

const u8 gText_lu_CGOptionName_Rematch_Interval[] = _("Update interval (steps)");
const u8 gText_lu_CGOptionHelp_Rematch_Interval[] = _("Control how often trainers potentially become available for a rematch, as measured in steps.\n\nDefault: 255 steps.");

const u8 gText_lu_CGOptionName_Rematch_Chance[] = _("Chance");
const u8 gText_lu_CGOptionHelp_Rematch_Chance[] = _("Control the likelihood that each previously battled trainer will become available for a rematch, every time the update interval elapses.\n\nDefault: 31%.");