BattleScript_EffectBide::
	attackcanceler
	ppreduce
	attackstringandanimation
	waitanimation
	orword gHitMarker, HITMARKER_CHARGING
	setbide
	goto BattleScript_MoveEnd

BattleScript_BideStoringEnergy::
	printstring STRINGID_PKMNSTORINGENERGY
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_BideAttack::
	attackcanceler
	setmoveeffect MOVE_EFFECT_CHARGING
	clearstatusfromeffect BS_ATTACKER
	printstring STRINGID_PKMNUNLEASHEDENERGY
	waitmessage B_WAIT_TIME_LONG
	accuracycheck BattleScript_MoveMissed, ACC_CURR_MOVE
	typecalc
	bicbyte gMoveResultFlags, MOVE_RESULT_SUPER_EFFECTIVE | MOVE_RESULT_NOT_VERY_EFFECTIVE
	copyword gBattleMoveDamage, sBIDE_DMG
	adjustsetdamage
	setbyte sB_ANIM_TURN, 1
	attackanimation
	waitanimation
	effectivenesssound
	hitanimation BS_TARGET
	waitstate
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	tryfaintmon BS_TARGET
	goto BattleScript_MoveEnd

BattleScript_BideNoEnergyToAttack::
	attackcanceler
	setmoveeffect MOVE_EFFECT_CHARGING
	clearstatusfromeffect BS_ATTACKER
	printstring STRINGID_PKMNUNLEASHEDENERGY
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_ButItFailed
