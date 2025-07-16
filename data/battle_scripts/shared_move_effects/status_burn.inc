BattleScript_EffectBurn::
	attackcanceler
	ppreduce
   @
   @ The checks run here should be kept consistent with SetMoveEffect's handler 
   @ for STATUS1_BURN. The Safeguard check should match SetMoveEffect's more 
   @ general behavior as well.
   @
	jumpifstatus2 BS_TARGET, STATUS2_SUBSTITUTE, BattleScript_Failed_AtkStringNoPPReduce
	jumpifstatus  BS_TARGET, STATUS1_BURN,       BattleScript_AlreadyBurned
	jumpiftype    BS_TARGET, TYPE_FIRE,          BattleScript_NotAffectedWithAtkString
	jumpifability BS_TARGET, ABILITY_WATER_VEIL, BattleScript_WaterVeilPrevents
	jumpifstatus  BS_TARGET, STATUS1_ANY,        BattleScript_Failed_AtkStringNoPPReduce
   @
	accuracycheck BattleScript_Failed_AtkStringNoPPReduce, ACC_CURR_MOVE
	jumpifsideaffecting BS_TARGET, SIDE_STATUS_SAFEGUARD, BattleScript_SafeguardProtectedWithAtkString
	attackstringandanimation
	waitanimation
	setmoveeffect MOVE_EFFECT_BURN
	seteffectprimary
	goto BattleScript_MoveEnd

BattleScript_AlreadyBurned::
	attackstring
	setalreadystatusedmoveattempt BS_ATTACKER
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNALREADYHASBURN
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_WaterVeilPrevents::
	attackstring
	copybyte gEffectBattler, gBattlerTarget
	setbyte cMULTISTRING_CHOOSER, B_MSG_ABILITY_PREVENTS_MOVE_STATUS
	call BattleScript_BRNPrevention
	goto BattleScript_MoveEnd
