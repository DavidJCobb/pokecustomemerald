BattleScript_EffectSafeguard::
	attackcanceler
	ppreduce
	setsafeguard
	goto BattleScript_PrintReflectLightScreenSafeguardString

BattleScript_SafeguardProtectedWithAtkString::
	attackstring
BattleScript_SafeguardProtected::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNUSEDSAFEGUARD
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_SafeguardEnds::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNSAFEGUARDEXPIRED
	waitmessage B_WAIT_TIME_LONG
	end2
