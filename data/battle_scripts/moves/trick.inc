BattleScript_EffectTrick::
	attackcanceler
	attackstring
	ppreduce
	jumpifstatus2 BS_TARGET, STATUS2_SUBSTITUTE, BattleScript_ButItFailed
	accuracycheck BattleScript_ButItFailed, ACC_CURR_MOVE
	tryswapitems BattleScript_ButItFailed
	attackanimation
	waitanimation
	printstring STRINGID_PKMNSWITCHEDITEMS
	waitmessage B_WAIT_TIME_LONG
	printfromtable gItemSwapStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
