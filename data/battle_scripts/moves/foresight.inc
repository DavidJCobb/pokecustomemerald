BattleScript_EffectForesight::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	setforesight
	attackanimation
	waitanimation
	printstring STRINGID_PKMNIDENTIFIED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
