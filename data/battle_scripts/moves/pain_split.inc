BattleScript_EffectPainSplit::
	attackcanceler
	ppreduce
	accuracycheck BattleScript_Failed_AtkStringNoPPReduce, NO_ACC_CALC_CHECK_LOCK_ON
	painsplitdmgcalc BattleScript_Failed_AtkStringNoPPReduce
	attackstringandanimation
	waitanimation
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	copyword gBattleMoveDamage, sPAINSPLIT_HP
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	printstring STRINGID_SHAREDPAIN
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
