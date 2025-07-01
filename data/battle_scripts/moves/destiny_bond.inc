BattleScript_EffectDestinyBond::
	attackcanceler
	attackstring
	ppreduce
	setdestinybond
	attackanimation
	waitanimation
	printstring STRINGID_PKMNTRYINGTOTAKEFOE
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_DestinyBondTakesLife::
	printstring STRINGID_PKMNTOOKFOE
	waitmessage B_WAIT_TIME_LONG
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_DAMAGE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	tryfaintmon BS_ATTACKER
	return
