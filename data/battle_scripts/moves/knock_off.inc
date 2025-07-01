BattleScript_EffectKnockOff::
	setmoveeffect MOVE_EFFECT_KNOCK_OFF
	goto BattleScript_EffectHit

BattleScript_KnockedOff::
	playanimation BS_TARGET, B_ANIM_ITEM_KNOCKOFF
	printstring STRINGID_PKMNKNOCKEDOFF
	waitmessage B_WAIT_TIME_LONG
	return
