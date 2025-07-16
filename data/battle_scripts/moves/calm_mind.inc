BattleScript_EffectCalmMind::
	attackcanceler
	ppreduce
	trystatchange (BIT_SPATK | BIT_SPDEF), (MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_DEFER_ALL_VISUALS), 1, STAT_CHANGE_CAUSE_MOVE_EFFECT_PRIMARY, BattleScript_CantRaiseMultipleStats
	attackstringandanimation
	waitanimation
	showstatchange
	goto BattleScript_MoveEnd
