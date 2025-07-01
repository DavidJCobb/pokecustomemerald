BattleScript_EffectSnatch::
	attackcanceler
	trysetsnatch BattleScript_FailedFromAtkString
	attackstring
	ppreduce
	attackanimation
	waitanimation
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNWAITSFORTARGET
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

@ Called by an attack canceller.
@
BattleScript_SnatchedMove::
	attackstring
	ppreduce
	snatchsetbattlers
	playanimation BS_TARGET, B_ANIM_SNATCH_MOVE
	printstring STRINGID_PKMNSNATCHEDMOVE
	waitmessage B_WAIT_TIME_LONG
	orword gHitMarker, HITMARKER_ATTACKSTRING_PRINTED | HITMARKER_NO_PPDEDUCT | HITMARKER_ALLOW_NO_PP
	swapattackerwithtarget
	return
