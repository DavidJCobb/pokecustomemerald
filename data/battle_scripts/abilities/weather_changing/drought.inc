BattleScript_DroughtActivates::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNSXINTENSIFIEDSUN
	waitstate
	playanimation BS_BATTLER_0, B_ANIM_SUN_CONTINUES
	call BattleScript_WeatherFormChanges
	end3
