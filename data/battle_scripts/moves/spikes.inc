BattleScript_EffectSpikes::
	attackcanceler
	trysetspikes BattleScript_FailedFromAtkString
	attackstring
	ppreduce
	attackanimation
	waitanimation
	printstring STRINGID_SPIKESSCATTERED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_SpikesOnAttacker::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_DAMAGE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	call BattleScript_PrintHurtBySpikes
	tryfaintmon BS_ATTACKER
	tryfaintmon_spikes BS_ATTACKER, BattleScript_SpikesOnAttackerFainted
	return

BattleScript_SpikesOnAttackerFainted::
	setbyte sGIVEEXP_STATE, 0
	getexp BS_ATTACKER
	moveendall
	goto BattleScript_HandleFaintedMon

BattleScript_SpikesOnTarget::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_DAMAGE
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	call BattleScript_PrintHurtBySpikes
	tryfaintmon BS_TARGET
	tryfaintmon_spikes BS_TARGET, BattleScript_SpikesOnTargetFainted
	return

BattleScript_SpikesOnTargetFainted::
	setbyte sGIVEEXP_STATE, 0
	getexp BS_TARGET
	moveendall
	goto BattleScript_HandleFaintedMon

BattleScript_SpikesOnFaintedBattler::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_DAMAGE
	healthbarupdate BS_FAINTED
	datahpupdate BS_FAINTED
	call BattleScript_PrintHurtBySpikes
	tryfaintmon BS_FAINTED
	tryfaintmon_spikes BS_FAINTED, BattleScript_SpikesOnFaintedBattlerFainted
	return

BattleScript_SpikesOnFaintedBattlerFainted::
	setbyte sGIVEEXP_STATE, 0
	getexp BS_FAINTED
	moveendall
	goto BattleScript_HandleFaintedMon

BattleScript_PrintHurtBySpikes::
	printstring STRINGID_PKMNHURTBYSPIKES
	waitmessage B_WAIT_TIME_LONG
	return
