BattleScript_EffectFacade::
	jumpifstatus BS_ATTACKER, STATUS1_POISON | STATUS1_BURN | STATUS1_PARALYSIS | STATUS1_TOXIC_POISON, BattleScript_FacadeDoubleDmg
	goto BattleScript_EffectHit

BattleScript_FacadeDoubleDmg::
	setbyte sDMG_MULTIPLIER, 2
	goto BattleScript_EffectHit
