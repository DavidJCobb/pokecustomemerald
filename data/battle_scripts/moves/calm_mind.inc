BattleScript_EffectCalmMind::
	attackcanceler
	ppreduce
	jumpifstat BS_ATTACKER, CMP_LESS_THAN, STAT_SPATK, MAX_STAT_STAGE, BattleScript_CalmMindDoMoveAnim
	jumpifstat BS_ATTACKER, CMP_EQUAL, STAT_SPDEF, MAX_STAT_STAGE, BattleScript_CantRaiseMultipleStats
BattleScript_CalmMindDoMoveAnim::
	attackstringandanimation
	waitanimation
	setbyte sSTAT_ANIM_PLAYED, FALSE
	playstatchangeanimation BS_ATTACKER, BIT_SPATK | BIT_SPDEF, 0
	setstatchanger STAT_SPATK, 1, FALSE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_CalmMindTrySpDef
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_STAT_WONT_INCREASE, BattleScript_CalmMindTrySpDef
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_CalmMindTrySpDef::
	setstatchanger STAT_SPDEF, 1, FALSE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_CalmMindEnd
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_STAT_WONT_INCREASE, BattleScript_CalmMindEnd
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_CalmMindEnd::
	goto BattleScript_MoveEnd
