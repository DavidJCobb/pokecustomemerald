BattleScript_EffectCosmicPower::
	attackcanceler
	ppreduce
	jumpifstat BS_ATTACKER, CMP_LESS_THAN, STAT_DEF, MAX_STAT_STAGE, BattleScript_CosmicPowerDoMoveAnim
	jumpifstat BS_ATTACKER, CMP_EQUAL, STAT_SPDEF, MAX_STAT_STAGE, BattleScript_CantRaiseMultipleStats
BattleScript_CosmicPowerDoMoveAnim::
	attackstringandanimation
	waitanimation
	setbyte sSTAT_ANIM_PLAYED, FALSE
	playstatchangeanimation BS_ATTACKER, BIT_DEF | BIT_SPDEF, 0
	setstatchanger STAT_DEF, 1, FALSE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_CosmicPowerTrySpDef
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_STAT_WONT_INCREASE, BattleScript_CosmicPowerTrySpDef
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_CosmicPowerTrySpDef::
	setstatchanger STAT_SPDEF, 1, FALSE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_CosmicPowerEnd
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_STAT_WONT_INCREASE, BattleScript_CosmicPowerEnd
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_CosmicPowerEnd::
	goto BattleScript_MoveEnd
