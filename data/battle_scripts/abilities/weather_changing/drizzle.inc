BattleScript_DrizzleActivates::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNMADEITRAIN
	waitstate
	playanimation BS_BATTLER_0, B_ANIM_RAIN_CONTINUES
	call BattleScript_WeatherFormChanges
	end3
