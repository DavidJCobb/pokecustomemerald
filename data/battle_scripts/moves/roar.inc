BattleScript_EffectRoar::
	attackcanceler
	ppreduce
	jumpifability BS_TARGET, ABILITY_SUCTION_CUPS, BattleScript_AbilityPreventsPhasingOut
	jumpifstatus3 BS_TARGET, STATUS3_ROOTED,       BattleScript_PrintMonIsRooted
	accuracycheck BattleScript_Failed_AtkStringNoPPReduce, NO_ACC_CALC_CHECK_LOCK_ON
	accuracycheck BattleScript_PrintMoveMissed_NoPPReduce, ACC_CURR_MOVE
	jumpifbattletype BATTLE_TYPE_ARENA, BattleScript_Failed_AtkStringNoPPReduce
   
   @ May invoke `BattleScript_SuccessForceOut`:
	forcerandomswitch BattleScript_ButItFailed

BattleScript_AbilityPreventsPhasingOut::
	attackstring
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNANCHORSITSELFWITH
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_PrintMonIsRooted::
	attackstring
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNANCHOREDITSELF
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_SuccessForceOut::
	attackstringandanimation
	waitanimation
	switchoutabilities BS_TARGET
	returntoball BS_TARGET
	waitstate
	jumpifbattletype BATTLE_TYPE_TRAINER, BattleScript_TrainerBattleForceOut
	setoutcomeonteleport BS_ATTACKER
	finishaction

BattleScript_TrainerBattleForceOut::
	getswitchedmondata BS_TARGET
	switchindataupdate BS_TARGET
	switchinanim BS_TARGET, FALSE
	waitstate
	printstring STRINGID_PKMNWASDRAGGEDOUT
	switchineffects BS_TARGET
	goto BattleScript_MoveEnd
