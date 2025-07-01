BattleScript_EffectSleep::
	attackcanceler
	ppreduce
	jumpifstatus2       BS_TARGET, STATUS2_SUBSTITUTE,    BattleScript_Failed_AtkStringNoPPReduce
	jumpifstatus        BS_TARGET, STATUS1_SLEEP,         BattleScript_AlreadyAsleep
	jumpifcantmakeasleep BattleScript_CantMakeAsleep
	jumpifstatus        BS_TARGET, STATUS1_ANY,           BattleScript_Failed_AtkStringNoPPReduce
	accuracycheck       BattleScript_Failed_AtkStringNoPPReduce, ACC_CURR_MOVE
	jumpifsideaffecting BS_TARGET, SIDE_STATUS_SAFEGUARD, BattleScript_SafeguardProtectedWithAtkString
	attackstringandanimation
	waitanimation
	setmoveeffect MOVE_EFFECT_SLEEP
	seteffectprimary
	goto BattleScript_MoveEnd

BattleScript_CantMakeAsleep::
	attackstring
	pause B_WAIT_TIME_SHORT
	printfromtable gUproarAwakeStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_AlreadyAsleep::
	attackstring
	setalreadystatusedmoveattempt BS_ATTACKER
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNALREADYASLEEP
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
   goto BattleScript_ButItFailed
