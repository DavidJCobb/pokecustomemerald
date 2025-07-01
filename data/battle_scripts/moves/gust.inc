BattleScript_EffectGust::
	jumpifnostatus3 BS_TARGET, STATUS3_ON_AIR, BattleScript_EffectHit
	orword gHitMarker, HITMARKER_IGNORE_ON_AIR
	setbyte sDMG_MULTIPLIER, 2
	goto BattleScript_EffectHit
