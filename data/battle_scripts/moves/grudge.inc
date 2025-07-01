BattleScript_EffectGrudge::
	attackcanceler
	ppreduce
	trysetgrudge BattleScript_Failed_AtkStringNoPPReduce
	attackstringandanimation
	waitanimation
	printstring STRINGID_PKMNWANTSGRUDGE
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

@ Called by `tryfaintmon`.
@
BattleScript_GrudgeTakesPp::
	printstring STRINGID_PKMNLOSTPPGRUDGE
	waitmessage B_WAIT_TIME_LONG
	return
