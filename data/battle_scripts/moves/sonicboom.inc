BattleScript_EffectSonicboom::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	ppreduce
	typecalc
	bicbyte gMoveResultFlags, MOVE_RESULT_SUPER_EFFECTIVE | MOVE_RESULT_NOT_VERY_EFFECTIVE
	setword gBattleMoveDamage, 20
	adjustsetdamage
	goto BattleScript_HitFromAtkAnimation
