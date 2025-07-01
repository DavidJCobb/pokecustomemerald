BattleScript_EffectDisable::
	attackcanceler
	ppreduce
	accuracycheck BattleScript_Failed_AtkStringNoPPReduce, ACC_CURR_MOVE
	disablelastusedattack BattleScript_Failed_AtkStringNoPPReduce
	attackstringandanimation
	waitanimation
	printstring STRINGID_PKMNMOVEWASDISABLED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_DisabledNoMore::
	printstring STRINGID_PKMNMOVEDISABLEDNOMORE
	waitmessage B_WAIT_TIME_LONG
	end2

@ Edge-case: a slower Pokemon uses Disable on a faster Pokemon, disabling 
@ the move that that Pokemon selected for this turn.
@
BattleScript_MoveUsedIsDisabled::
	printstring STRINGID_PKMNMOVEISDISABLED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
