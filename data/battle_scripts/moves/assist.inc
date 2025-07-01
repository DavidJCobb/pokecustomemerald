BattleScript_EffectAssist::
	attackcanceler
	assistattackselect BattleScript_FailedFromAtkString
	attackstringandanimation
	waitanimation
	setbyte sB_ANIM_TURN, 0
	setbyte sB_ANIM_TARGETS_HIT, 0
	jumptocalledmove TRUE
