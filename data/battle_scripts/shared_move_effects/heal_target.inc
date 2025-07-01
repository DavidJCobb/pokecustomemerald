@
@ Heals the target by adding gBattleMoveDamage to its health. The value 
@ of gBattleMoveDamage should be a negative number.
@
BattleScript_HealTarget::
	attackanimation
	waitanimation
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	printstring STRINGID_PKMNREGAINEDHEALTH
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
