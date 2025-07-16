BattleScript_EffectTickle::
	attackcanceler
	ppreduce
	trystatchange (BIT_ATK | BIT_DEF), (STAT_CHANGE_DEFER_ALL_VISUALS), -1, STAT_CHANGE_CAUSE_MOVE_EFFECT_PRIMARY, BattleScript_CantLowerMultipleStats
	accuracycheck BattleScript_Failed_AtkStringNoPPReduce, ACC_CURR_MOVE
	attackstringandanimation
	waitanimation
	showstatchange
	goto BattleScript_MoveEnd
