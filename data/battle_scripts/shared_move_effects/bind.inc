BattleScript_EffectTrap::
	jumpifnotmove MOVE_WHIRLPOOL, BattleScript_DoWrapEffect
	jumpifnostatus3 BS_TARGET, STATUS3_UNDERWATER, BattleScript_DoWrapEffect
	orword gHitMarker, HITMARKER_IGNORE_UNDERWATER
	setbyte sDMG_MULTIPLIER, 2
BattleScript_DoWrapEffect::
	setmoveeffect MOVE_EFFECT_WRAP
	goto BattleScript_EffectHit
