BattleScript_EffectParalyze::
	attackcanceler
	ppreduce
	jumpifability BS_TARGET, ABILITY_LIMBER,     BattleScript_LimberProtected
	jumpifstatus2 BS_TARGET, STATUS2_SUBSTITUTE, BattleScript_Failed_AtkStringNoPPReduce
	typecalc
	jumpifmovehadnoeffect BattleScript_Failed_AtkStringNoPPReduce
	jumpifstatus  BS_TARGET, STATUS1_PARALYSIS,  BattleScript_AlreadyParalyzed
	jumpifstatus  BS_TARGET, STATUS1_ANY,        BattleScript_Failed_AtkStringNoPPReduce
	accuracycheck BattleScript_Failed_AtkStringNoPPReduce, ACC_CURR_MOVE
	jumpifsideaffecting BS_TARGET, SIDE_STATUS_SAFEGUARD, BattleScript_SafeguardProtectedWithAtkString
	attackstringandanimation
	waitanimation
	setmoveeffect MOVE_EFFECT_PARALYSIS
	seteffectprimary
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_AlreadyParalyzed::
	attackstring
	setalreadystatusedmoveattempt BS_ATTACKER
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNISALREADYPARALYZED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_LimberProtected::
	attackstring
	copybyte gEffectBattler, gBattlerTarget
	setbyte cMULTISTRING_CHOOSER, B_MSG_ABILITY_PREVENTS_MOVE_STATUS
	call BattleScript_PRLZPrevention
	goto BattleScript_MoveEnd
