BattleScript_EffectDragonDance::
	attackcanceler
	attackstring
	ppreduce
	jumpifstat BS_ATTACKER, CMP_LESS_THAN, STAT_ATK, MAX_STAT_STAGE, BattleScript_DragonDanceDoMoveAnim
	jumpifstat BS_ATTACKER, CMP_EQUAL, STAT_SPEED, MAX_STAT_STAGE, BattleScript_CantRaiseMultipleStats
BattleScript_DragonDanceDoMoveAnim::
	attackanimation
	waitanimation
	setbyte sSTAT_ANIM_PLAYED, FALSE
	playstatchangeanimation BS_ATTACKER, BIT_ATK | BIT_SPEED, 0
	setstatchanger STAT_ATK, 1, FALSE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_DragonDanceTrySpeed
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_STAT_WONT_INCREASE, BattleScript_DragonDanceTrySpeed
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_DragonDanceTrySpeed::
	setstatchanger STAT_SPEED, 1, FALSE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_DragonDanceEnd
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_STAT_WONT_INCREASE, BattleScript_DragonDanceEnd
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_DragonDanceEnd::
	goto BattleScript_MoveEnd
