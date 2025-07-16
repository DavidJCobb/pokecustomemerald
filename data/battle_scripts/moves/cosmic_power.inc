BattleScript_EffectCosmicPower::
	attackcanceler
	ppreduce
	jumpifstat BS_ATTACKER, CMP_LESS_THAN, STAT_DEF, MAX_STAT_STAGE, BattleScript_CosmicPowerDoMoveAnim
	jumpifstat BS_ATTACKER, CMP_EQUAL, STAT_SPDEF, MAX_STAT_STAGE, BattleScript_CantRaiseMultipleStats
BattleScript_CosmicPowerDoMoveAnim::
	attackstringandanimation
	waitanimation
   trystatchange (BIT_DEF | BIT_SPDEF), (MOVE_EFFECT_AFFECTS_USER), 1, STAT_CHANGE_CAUSE_MOVE_EFFECT_PRIMARY, 0
BattleScript_CosmicPowerEnd::
	goto BattleScript_MoveEnd
