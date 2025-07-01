BattleScript_EffectCurse::
	jumpiftype2 BS_ATTACKER, TYPE_GHOST, BattleScript_GhostCurse
	attackcanceler
	attackstring
	ppreduce
	jumpifstat BS_ATTACKER, CMP_GREATER_THAN, STAT_SPEED, MIN_STAT_STAGE, BattleScript_CurseTrySpeed
	jumpifstat BS_ATTACKER, CMP_NOT_EQUAL, STAT_ATK, MAX_STAT_STAGE, BattleScript_CurseTrySpeed
	jumpifstat BS_ATTACKER, CMP_EQUAL, STAT_DEF, MAX_STAT_STAGE, BattleScript_ButItFailed
BattleScript_CurseTrySpeed::
	copybyte gBattlerTarget, gBattlerAttacker
	setbyte sB_ANIM_TURN, 1
	attackanimation
	waitanimation
	setstatchanger STAT_SPEED, 1, TRUE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_CurseTryAttack
	printfromtable gStatDownStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_CurseTryAttack::
	setstatchanger STAT_ATK, 1, FALSE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_CurseTryDefense
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_CurseTryDefense::
	setstatchanger STAT_DEF, 1, FALSE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_CurseEnd
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_CurseEnd::
	goto BattleScript_MoveEnd
BattleScript_GhostCurse::
	jumpifbytenotequal gBattlerAttacker, gBattlerTarget, BattleScript_DoGhostCurse
	getmovetarget BS_ATTACKER
BattleScript_DoGhostCurse::
	attackcanceler
	attackstring
	ppreduce
	jumpifstatus2 BS_TARGET, STATUS2_SUBSTITUTE, BattleScript_ButItFailed
	accuracycheck BattleScript_ButItFailed, NO_ACC_CALC_CHECK_LOCK_ON
	cursetarget BattleScript_ButItFailed
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	setbyte sB_ANIM_TURN, 0
	attackanimation
	waitanimation
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printstring STRINGID_PKMNLAIDCURSE
	waitmessage B_WAIT_TIME_LONG
	tryfaintmon BS_ATTACKER
	goto BattleScript_MoveEnd
