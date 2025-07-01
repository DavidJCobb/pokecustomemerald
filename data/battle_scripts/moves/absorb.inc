BattleScript_EffectAbsorb::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	ppreduce
	critcalc
	damagecalc
	typecalc
	adjustnormaldamage
	attackstringandanimation
	waitanimation
	effectivenesssound
	hitanimation BS_TARGET
	waitstate
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	critmessage
	waitmessage B_WAIT_TIME_LONG
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	negativedamage
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	jumpifability BS_TARGET, ABILITY_LIQUID_OOZE, BattleScript_AbsorbLiquidOoze
	setbyte cMULTISTRING_CHOOSER, B_MSG_ABSORB
	goto BattleScript_AbsorbUpdateHp
BattleScript_AbsorbLiquidOoze::
	manipulatedamage DMG_CHANGE_SIGN
	setbyte cMULTISTRING_CHOOSER, B_MSG_ABSORB_OOZE
BattleScript_AbsorbUpdateHp::
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	jumpifmovehadnoeffect BattleScript_AbsorbTryFainting
	printfromtable gAbsorbDrainStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_AbsorbTryFainting::
	tryfaintmon BS_ATTACKER
	tryfaintmon BS_TARGET
	goto BattleScript_MoveEnd
