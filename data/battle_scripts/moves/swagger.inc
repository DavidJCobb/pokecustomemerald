BattleScript_EffectSwagger::
	attackcanceler
	jumpifstatus2 BS_TARGET, STATUS2_SUBSTITUTE, BattleScript_MakeMoveMissed
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	ppreduce
	jumpifconfusedandstatmaxed STAT_ATK, BattleScript_Failed_AtkStringNoPPReduce
	attackstringandanimation
	waitanimation
	trystatchange (BIT_ATK), (0), 2, STAT_CHANGE_CAUSE_MOVE_EFFECT_PRIMARY, 0
BattleScript_SwaggerTryConfuse::
	jumpifability BS_TARGET, ABILITY_OWN_TEMPO, BattleScript_OwnTempoPrevents
	jumpifsideaffecting BS_TARGET, SIDE_STATUS_SAFEGUARD, BattleScript_SafeguardProtected
	setmoveeffect MOVE_EFFECT_CONFUSION
	seteffectprimary
	goto BattleScript_MoveEnd
