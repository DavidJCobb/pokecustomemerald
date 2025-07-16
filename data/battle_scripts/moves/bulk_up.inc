BattleScript_EffectBulkUp::
	attackcanceler
	ppreduce
	jumpifstat BS_ATTACKER, CMP_LESS_THAN, STAT_ATK, MAX_STAT_STAGE, BattleScript_BulkUpDoMoveAnim
	jumpifstat BS_ATTACKER, CMP_EQUAL, STAT_DEF, MAX_STAT_STAGE, BattleScript_CantRaiseMultipleStats
BattleScript_BulkUpDoMoveAnim::
	attackstringandanimation
	waitanimation
	trystatchange (BIT_ATK | BIT_DEF), (MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_SUPPRESS_FAILURE_MESSAGES), 1, STAT_CHANGE_CAUSE_MOVE_EFFECT_PRIMARY, 0
BattleScript_BulkUpEnd::
	goto BattleScript_MoveEnd
