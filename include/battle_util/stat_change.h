#ifndef GUARD_LU_BATTLE_UTIL_STAT_CHANGES_H
#define GUARD_LU_BATTLE_UTIL_STAT_CHANGES_H

enum CannotDecreaseAnyStatsReason {
   CANNOTDECREASEANYSTATSREASON_NONE,
   CANNOTDECREASEANYSTATSREASON_ABILITY, // Subject's ability blocks all stat losses.
   CANNOTDECREASEANYSTATSREASON_MIST,
   CANNOTDECREASEANYSTATSREASON_PROTECT,
};

enum CannotDecreaseAnyStatsReason CheckDecreaseBattlerAnyStats(
   u8    battler,
   u16   move,
   bool8 certain,
   bool8 bypass_protect
);

enum CannotDecreaseSpecificStatReason {
   CANNOTDECREASESPECIFICSTATREASON_NONE,
   CANNOTDECREASESPECIFICSTATREASON_ABILITY, // Subject's ability guards a specific stat.
   CANNOTDECREASESPECIFICSTATREASON_BOUNDED, // Subject's stat cannot go lower.
};

enum CannotDecreaseSpecificStatReason CheckDecreaseBattlerStat(
   u8    battler,
   u8    stat,
   bool8 certain
);

#endif