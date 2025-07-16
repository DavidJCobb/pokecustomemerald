BattleScript_EffectCalmMind::
	attackcanceler
	ppreduce
	jumpifstat BS_ATTACKER, CMP_LESS_THAN, STAT_SPATK, MAX_STAT_STAGE, BattleScript_CalmMindDoMoveAnim
	jumpifstat BS_ATTACKER, CMP_EQUAL, STAT_SPDEF, MAX_STAT_STAGE, BattleScript_CantRaiseMultipleStats
BattleScript_CalmMindDoMoveAnim::
	attackstringandanimation
	waitanimation
	trystatchange (BIT_SPATK | BIT_SPDEF), (MOVE_EFFECT_AFFECTS_USER), 1, STAT_CHANGE_CAUSE_MOVE_EFFECT_PRIMARY, 0
BattleScript_CalmMindEnd::
	goto BattleScript_MoveEnd
