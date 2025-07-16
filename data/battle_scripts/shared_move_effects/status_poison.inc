@ Script for a move which poisons the target.
@
BattleScript_EffectPoison::
	setmoveeffect MOVE_EFFECT_POISON
   goto BattleScript_EffectPoisonAnyVariety

@ Script for a move which badly poisons the target.
@
BattleScript_EffectToxic::
	setmoveeffect MOVE_EFFECT_TOXIC
   goto BattleScript_EffectPoisonAnyVariety

BattleScript_EffectPoisonAnyVariety::
	attackcanceler
	ppreduce
   @
   @ The checks run here should be kept consistent with SetMoveEffect's handler 
   @ for STATUS1_POISON. The Safeguard check should match SetMoveEffect's more 
   @ general behavior as well.
   @
	jumpifability BS_TARGET, ABILITY_IMMUNITY,     BattleScript_ImmunityProtected
	jumpifstatus2 BS_TARGET, STATUS2_SUBSTITUTE,   BattleScript_Failed_AtkStringNoPPReduce
	jumpifstatus  BS_TARGET, STATUS1_POISON,       BattleScript_AlreadyPoisoned
	jumpifstatus  BS_TARGET, STATUS1_TOXIC_POISON, BattleScript_AlreadyPoisoned
	jumpifstatus  BS_TARGET, STATUS1_ANY,          BattleScript_Failed_AtkStringNoPPReduce
	jumpiftype    BS_TARGET, TYPE_POISON,          BattleScript_NotAffectedWithAtkString
	jumpiftype    BS_TARGET, TYPE_STEEL,           BattleScript_NotAffectedWithAtkString
   @
	accuracycheck BattleScript_Failed_AtkStringNoPPReduce, ACC_CURR_MOVE
	jumpifsideaffecting BS_TARGET, SIDE_STATUS_SAFEGUARD, BattleScript_SafeguardProtectedWithAtkString
	attackstringandanimation
	waitanimation
	seteffectprimary
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_AlreadyPoisoned::
	attackstring
	setalreadystatusedmoveattempt BS_ATTACKER
	pause B_WAIT_TIME_LONG
	printstring STRINGID_PKMNALREADYPOISONED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_ImmunityProtected::
	attackstring
	copybyte gEffectBattler, gBattlerTarget
	setbyte cMULTISTRING_CHOOSER, B_MSG_ABILITY_PREVENTS_MOVE_STATUS
	call BattleScript_PSNPrevention
	goto BattleScript_MoveEnd
