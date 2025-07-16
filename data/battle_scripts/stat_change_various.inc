

BattleScript_CantLowerMultipleStats::
	attackstring
	pause B_WAIT_TIME_SHORT
	showstatchange
	orbyte gMoveResultFlags, MOVE_RESULT_FAILED
	goto BattleScript_MoveEnd

BattleScript_CantRaiseMultipleStats::
	attackstring
	pause B_WAIT_TIME_SHORT
	showstatchange
	orbyte gMoveResultFlags, MOVE_RESULT_FAILED
	goto BattleScript_MoveEnd

@ Randomly self-applied secondary effect of Ancientpower.
BattleScript_AllStatsUp::
	jumpifstat BS_ATTACKER, CMP_LESS_THAN, STAT_ATK, MAX_STAT_STAGE, BattleScript_AllStatsUpAtk
	jumpifstat BS_ATTACKER, CMP_LESS_THAN, STAT_DEF, MAX_STAT_STAGE, BattleScript_AllStatsUpAtk
	jumpifstat BS_ATTACKER, CMP_LESS_THAN, STAT_SPEED, MAX_STAT_STAGE, BattleScript_AllStatsUpAtk
	jumpifstat BS_ATTACKER, CMP_LESS_THAN, STAT_SPATK, MAX_STAT_STAGE, BattleScript_AllStatsUpAtk
	jumpifstat BS_ATTACKER, CMP_EQUAL, STAT_SPDEF, MAX_STAT_STAGE, BattleScript_AllStatsUpRet
BattleScript_AllStatsUpAtk::
	trystatchange (BIT_ATK | BIT_DEF | BIT_SPEED | BIT_SPATK | BIT_SPDEF), (MOVE_EFFECT_AFFECTS_USER), 1, STAT_CHANGE_CAUSE_MOVE_EFFECT_SECONDARY, 0
BattleScript_AllStatsUpRet::
	return

@ Self-applied secondary effect of Superpower.
BattleScript_AtkDefDown::
	trystatchange (BIT_ATK | BIT_DEF), (MOVE_EFFECT_AFFECTS_USER | MOVE_EFFECT_CERTAIN | STAT_CHANGE_SUPPRESS_FAILURE_MESSAGES), -1, STAT_CHANGE_CAUSE_MOVE_EFFECT_SECONDARY, 0
	return

@ Self-applied secondary effect of Overheat.
BattleScript_SAtkDown2::
	trystatchange (BIT_SPATK), (MOVE_EFFECT_AFFECTS_USER | MOVE_EFFECT_CERTAIN | STAT_CHANGE_SUPPRESS_FAILURE_MESSAGES), -2, STAT_CHANGE_CAUSE_MOVE_EFFECT_SECONDARY, 0
	return
