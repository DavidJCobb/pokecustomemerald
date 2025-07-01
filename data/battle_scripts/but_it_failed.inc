BattleScript_FailedFromAtkString::
	attackstring
BattleScript_FailedFromPpReduce::
	ppreduce
BattleScript_ButItFailed::
	pause B_WAIT_TIME_SHORT
	orbyte gMoveResultFlags, MOVE_RESULT_FAILED
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_Failed_AtkStringNoPPReduce::
	attackstring
   goto BattleScript_ButItFailed
