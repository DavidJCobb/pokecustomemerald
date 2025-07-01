BattleScript_EffectGrudge::
	attackcanceler
	attackstring
	ppreduce
	trysetgrudge BattleScript_ButItFailed
	attackanimation
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
