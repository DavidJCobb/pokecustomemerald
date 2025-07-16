BattleScript_EffectDragonDance::
	attackcanceler
	ppreduce
	jumpifstat BS_ATTACKER, CMP_LESS_THAN, STAT_ATK, MAX_STAT_STAGE, BattleScript_DragonDanceSuccess
	jumpifstat BS_ATTACKER, CMP_EQUAL, STAT_SPEED, MAX_STAT_STAGE, BattleScript_DragonDanceFail
BattleScript_DragonDanceSuccess::
	attackstringandanimation
	waitanimation
	trystatchange (BIT_ATK | BIT_SPEED), (MOVE_EFFECT_AFFECTS_USER), 1, STAT_CHANGE_CAUSE_MOVE_EFFECT_PRIMARY, 0
BattleScript_DragonDanceEnd::
	goto BattleScript_MoveEnd

BattleScript_DragonDanceFail::
	attackstring
   goto BattleScript_CantRaiseMultipleStats
