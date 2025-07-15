#ifndef GUARD_LU_BATTLE_MESSAGES_STAT_CHANGES_H
#define GUARD_LU_BATTLE_MESSAGES_STAT_CHANGES_H

// Write a human-readable list of stats into the destination string, 
// and return a pointer to the end of the written content.
//
// Output examples:
//    "ATTACK"
//    "ATTACK and DEFENSE"
//    "ATTACK, DEFENSE, and SPEED"
//    "ATTACK, DEFENSE, SPEED, and evasiveness"
// 
u8* WriteStatNamesInto(u8* dst, u8 stats);

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