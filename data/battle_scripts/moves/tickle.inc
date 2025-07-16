BattleScript_EffectTickle::
	attackcanceler
	ppreduce
	jumpifstat BS_TARGET, CMP_GREATER_THAN, STAT_ATK, MIN_STAT_STAGE, BattleScript_TickleDoMoveAnim
	jumpifstat BS_TARGET, CMP_EQUAL, STAT_DEF, MIN_STAT_STAGE, BattleScript_TickleFail
BattleScript_TickleDoMoveAnim::
	accuracycheck BattleScript_ButItFailed, ACC_CURR_MOVE
	attackstringandanimation
	waitanimation
	trystatchange (BIT_ATK | BIT_DEF), (STAT_CHANGE_SUPPRESS_FAILURE_MESSAGES), -1, STAT_CHANGE_CAUSE_MOVE_EFFECT_PRIMARY, BattleScript_TickleEnd
BattleScript_TickleEnd::
	goto BattleScript_MoveEnd

BattleScript_TickleFail::
	attackstring
	goto BattleScript_CantLowerMultipleStats