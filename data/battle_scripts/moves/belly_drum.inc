BattleScript_EffectBellyDrum::
	attackcanceler
	ppreduce
	maxattackhalvehp BattleScript_Failed_AtkStringNoPPReduce
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	attackstringandanimation
	waitanimation
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printstring STRINGID_PKMNCUTHPMAXEDATTACK
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
