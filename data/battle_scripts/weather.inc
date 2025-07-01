
BattleScript_RainContinuesOrEnds::
	@
	@ The Battle Ambient Weather feature plays the weather animation on a
	@ loop during action selection, so if the string we are going to show 
	@ is "Rain continues to fall," then just skip it.
	@
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_RAIN_CONTINUES, BattleScript_RainContinuesOrEndsEnd
	@
	printfromtable gRainContinuesStringIds
	waitmessage B_WAIT_TIME_LONG
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_RAIN_STOPPED, BattleScript_RainContinuesOrEndsEnd
	playanimation BS_ATTACKER, B_ANIM_RAIN_CONTINUES
BattleScript_RainContinuesOrEndsEnd::
	end2

BattleScript_DamagingWeatherContinues::
	@
	@ The Battle Ambient Weather feature plays the weather animation on a
	@ loop during action selection. This means that we should skip the 
	@ initial string here, and we should only play the weather animation 
	@ here if any Pokemon takes damage from it (and we should only play 
	@ it for the first Pokemon that takes damage).
	@
	@ Since weather bypasses Bide, we can (ab)use HITMARKER_IGNORE_BIDE 
	@ as a sentinel value. We will explicitly clear it at the start of 
	@ our code, so we can check if it was ever set before we play the 
	@ animation.
	@
	@ -------------------------------------------------------------------
	@
	@ printfromtable gSandStormHailContinuesStringIds
	@ waitmessage B_WAIT_TIME_LONG
	@ playanimation_var BS_ATTACKER, sB_ANIM_ARG1
	bicword gHitMarker, HITMARKER_IGNORE_BIDE
	setbyte gBattleCommunication, 0
BattleScript_DamagingWeatherLoop::
	copyarraywithindex gBattlerAttacker, gBattlerByTurnOrder, gBattleCommunication, 1
	weatherdamage
	jumpifword CMP_EQUAL, gBattleMoveDamage, 0, BattleScript_DamagingWeatherLoopIncrement
	@
	@ Here is where we will conditionally play the weather animation.
	@
	jumpifword CMP_COMMON_BITS, gHitMarker, HITMARKER_IGNORE_BIDE, BattleScript_DamagingWeather_AfterAnimation
	playanimation_var BS_ATTACKER, sB_ANIM_ARG1
	waitanimation
BattleScript_DamagingWeather_AfterAnimation:
	printfromtable gSandStormHailDmgStringIds
	waitmessage B_WAIT_TIME_LONG
	orword gHitMarker, HITMARKER_IGNORE_BIDE | HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_DAMAGE | HITMARKER_GRUDGE
	effectivenesssound
	hitanimation BS_ATTACKER
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	tryfaintmon BS_ATTACKER
	@
	@ Hitmarkers generally only last once, getting cleared the next time 
	@ they are checked. This is true for the "ignore Bide" hitmarker, so 
	@ we need to set it again, as part of (ab)using it as a sentinel value.
	@
	orword gHitMarker, HITMARKER_IGNORE_BIDE
	@
	checkteamslost BattleScript_DamagingWeatherLoopIncrement
BattleScript_DamagingWeatherLoopIncrement::
	jumpifbyte CMP_NOT_EQUAL, gBattleOutcome, 0, BattleScript_DamagingWeatherContinuesEnd
	addbyte gBattleCommunication, 1
	jumpifbytenotequal gBattleCommunication, gBattlersCount, BattleScript_DamagingWeatherLoop
BattleScript_DamagingWeatherContinuesEnd::
	bicword gHitMarker, HITMARKER_IGNORE_BIDE | HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_DAMAGE | HITMARKER_GRUDGE
	end2

BattleScript_SandStormHailEnds::
	printfromtable gSandStormHailEndStringIds
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_SunlightContinues::
	@
	@ We can stub out this script, since the Battle Ambient Weather feature 
	@ plays the weather animation on a loop during action selection.
	@
	@ ---------------------------------------------------------------------
	@
	@ printstring STRINGID_SUNLIGHTSTRONG
	@ waitmessage B_WAIT_TIME_LONG
	@ playanimation BS_ATTACKER, B_ANIM_SUN_CONTINUES
	end2

BattleScript_SunlightFaded::
	printstring STRINGID_SUNLIGHTFADED
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_OverworldWeatherStarts::
	printfromtable gWeatherStartsStringIds
	waitmessage B_WAIT_TIME_LONG
	playanimation_var BS_ATTACKER, sB_ANIM_ARG1
	end3
