BattleScript_EffectRapidSpin::
	setmoveeffect MOVE_EFFECT_RAPIDSPIN | MOVE_EFFECT_AFFECTS_USER | MOVE_EFFECT_CERTAIN
	goto BattleScript_EffectHit

@ Called by MOVE_EFFECT_RAPIDSPIN handler.
@
BattleScript_RapidSpinAway::
	rapidspinfree
	return
