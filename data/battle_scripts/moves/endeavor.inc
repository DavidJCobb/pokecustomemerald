BattleScript_EffectEndeavor::
	attackcanceler
	ppreduce
	setdamagetohealthdifference BattleScript_Failed_AtkStringNoPPReduce
	copyword gHpDealt, gBattleMoveDamage
	accuracycheck BattleScript_PrintMoveMissed_NoPPReduce, ACC_CURR_MOVE
	typecalc
	jumpifmovehadnoeffect BattleScript_HitFromAtkAnimation
	bicbyte gMoveResultFlags, MOVE_RESULT_SUPER_EFFECTIVE | MOVE_RESULT_NOT_VERY_EFFECTIVE
	copyword gBattleMoveDamage, gHpDealt
	adjustsetdamage
	goto BattleScript_HitFromAtkAnimation
