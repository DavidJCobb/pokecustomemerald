BattleScript_EffectRage::
	attackcanceler
	accuracycheck BattleScript_RageMiss, ACC_CURR_MOVE
	setmoveeffect MOVE_EFFECT_RAGE
	seteffectprimary
	setmoveeffect 0
	goto BattleScript_HitFromAtkString
BattleScript_RageMiss::
	setmoveeffect MOVE_EFFECT_RAGE
	clearstatusfromeffect BS_ATTACKER
	goto BattleScript_PrintMoveMissed

@ Called by MOVEEND_RAGE handler.
@
BattleScript_RageIsBuilding::
	printstring STRINGID_PKMNRAGEBUILDING
	waitmessage B_WAIT_TIME_LONG
	return
