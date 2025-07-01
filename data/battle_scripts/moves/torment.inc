BattleScript_EffectTorment::
	attackcanceler
	attackstring
	ppreduce
	accuracycheck BattleScript_ButItFailed, ACC_CURR_MOVE
	settorment BattleScript_ButItFailed
	attackanimation
	waitanimation
	printstring STRINGID_PKMNSUBJECTEDTOTORMENT
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
