BattleScript_EffectSkullBash::
	jumpifstatus2 BS_ATTACKER, STATUS2_MULTIPLETURNS, BattleScript_TwoTurnMovesSecondTurn
	jumpifword CMP_COMMON_BITS, gHitMarker, HITMARKER_NO_ATTACKSTRING, BattleScript_TwoTurnMovesSecondTurn
	setbyte sTWOTURN_STRINGID, B_MSG_TURN1_SKULL_BASH
	call BattleScriptFirstChargingTurn
	trystatchange (BIT_DEF), (MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_SUPPRESS_FAILURE_MESSAGES), 1, STAT_CHANGE_CAUSE_MOVE_EFFECT_SECONDARY, 0
BattleScript_SkullBashEnd::
	goto BattleScript_MoveEnd
