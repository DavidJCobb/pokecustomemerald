BattleScript_EffectFocusEnergy::
	attackcanceler
	ppreduce
	jumpifstatus2 BS_ATTACKER, STATUS2_FOCUS_ENERGY, BattleScript_Failed_AtkStringNoPPReduce
	setfocusenergy
	attackstringandanimation
	waitanimation
	printfromtable gFocusEnergyUsedStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
