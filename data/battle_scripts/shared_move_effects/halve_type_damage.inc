BattleScript_EffectHalveTypeDamage::
	attackcanceler
	ppreduce
	settypebasedhalvers BattleScript_Failed_AtkStringNoPPReduce
	attackstringandanimation
	waitanimation
	printfromtable gSportsUsedStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
