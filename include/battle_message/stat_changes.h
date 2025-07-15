#ifndef GUARD_LU_BATTLE_MESSAGES_STAT_CHANGES_H
#define GUARD_LU_BATTLE_MESSAGES_STAT_CHANGES_H

#include "gba/types.h"

// Store a list of stat names into gBattleTextBuff1 and friends in 
// an optimized format. Writing them as a string won't fit, so 
// this instead takes advantage of special-case format codes that 
// are available only in those buffers.
void BufferStatList(u8* battle_text_buffer, u8 stats);

// Write a human-readable string describing the magnitude of a stat 
// change into the destination string, and return a pointer to the 
// end of the written content.
//
// Outputs: "rose", "sharply rose", "fell", "harshly fell"
u8* WriteStatMagnitudeInto(u8* dst, s8 magnitude);

enum StatChangeMessage_AllStatsBlockedBy {
   STATCHANGEMESSAGE_ALL_STATS_NOT_BLOCKED = 0,
   STATCHANGEMESSAGE_ALLSTATSBLOCKEDBY_ABILITY,
   STATCHANGEMESSAGE_ALLSTATSBLOCKEDBY_MIST,
   STATCHANGEMESSAGE_ALLSTATSBLOCKEDBY_PROTECT,
};

enum StatChangeMessage_AnyStatBlockedBy {
   STATCHANGEMESSAGE_ANYSTATBLOCKEDBY_ABILITY,
   STATCHANGEMESSAGE_ANYSTATBLOCKEDBY_BOUNDED,
};

enum StatChangeMessage_Cause {
   STATCHANGEMESSAGE_CAUSE_GENERAL,
   STATCHANGEMESSAGE_CAUSE_ABILITY,
   STATCHANGEMESSAGE_CAUSE_ITEM_HELD,
   STATCHANGEMESSAGE_CAUSE_ITEM_USED,
   STATCHANGEMESSAGE_CAUSE_SIDEEFFECT,
};

void BufferStatChangeSuccessMessage(
   u8 battler_cause,
   u8 battler_affected,
   u8 change_cause,
   u8 stats,
   s8 magnitude
);
void BufferStatChangeAllFailMessage(
   u8 battler_cause,
   u8 battler_affected,
   u8 change_cause,
   enum StatChangeMessage_AllStatsBlockedBy
);
void BufferStatChangeAnyFailMessage(
   u8 battler_cause,
   u8 battler_affected,
   u8 change_cause,
   u8 stats,
   s8 magnitude,
   enum StatChangeMessage_AnyStatBlockedBy
);

#endif