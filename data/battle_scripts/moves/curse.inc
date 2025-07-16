BattleScript_EffectCurse::
	jumpiftype2 BS_ATTACKER, TYPE_GHOST, BattleScript_GhostCurse
	attackcanceler
	ppreduce
	jumpifstat BS_ATTACKER, CMP_GREATER_THAN, STAT_SPEED, MIN_STAT_STAGE, BattleScript_Curse_Success
	jumpifstat BS_ATTACKER, CMP_NOT_EQUAL, STAT_ATK, MAX_STAT_STAGE, BattleScript_Curse_Success
	jumpifstat BS_ATTACKER, CMP_EQUAL, STAT_DEF, MAX_STAT_STAGE, BattleScript_Failed_AtkStringNoPPReduce
BattleScript_Curse_Success::
	copybyte gBattlerTarget, gBattlerAttacker
	setbyte sB_ANIM_TURN, 1
	attackstringandanimation
	waitanimation
	trystatchange (BIT_SPEED), (MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_SUPPRESS_ANIMATIONS), -1, STAT_CHANGE_CAUSE_MOVE_EFFECT_PRIMARY, 0
	trystatchange (BIT_ATK | BIT_DEF), (MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_SUPPRESS_ANIMATIONS), 1, STAT_CHANGE_CAUSE_MOVE_EFFECT_PRIMARY, 0
	goto BattleScript_MoveEnd

BattleScript_GhostCurse::
	jumpifbytenotequal gBattlerAttacker, gBattlerTarget, BattleScript_DoGhostCurse
	getmovetarget BS_ATTACKER
BattleScript_DoGhostCurse::
	attackcanceler
	ppreduce
	jumpifstatus2 BS_TARGET, STATUS2_SUBSTITUTE, BattleScript_Failed_AtkStringNoPPReduce
	accuracycheck BattleScript_Failed_AtkStringNoPPReduce, NO_ACC_CALC_CHECK_LOCK_ON
	cursetarget BattleScript_Failed_AtkStringNoPPReduce
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	setbyte sB_ANIM_TURN, 0
	attackstringandanimation
	waitanimation
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printstring STRINGID_PKMNLAIDCURSE
	waitmessage B_WAIT_TIME_LONG
	tryfaintmon BS_ATTACKER
	goto BattleScript_MoveEnd
