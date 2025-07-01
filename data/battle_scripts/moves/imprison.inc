BattleScript_EffectImprison::
	attackcanceler
	attackstring
	ppreduce
	tryimprison BattleScript_ButItFailed
	attackanimation
	waitanimation
	printstring STRINGID_PKMNSEALEDOPPONENTMOVE
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
