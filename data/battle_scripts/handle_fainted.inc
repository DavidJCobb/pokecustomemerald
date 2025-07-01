BattleScript_HandleFaintedMon::
	checkteamslost BattleScript_LinkHandleFaintedMonMultiple
	jumpifbyte CMP_NOT_EQUAL, gBattleOutcome, 0, BattleScript_FaintedMonEnd
	jumpifbattletype BATTLE_TYPE_TRAINER, BattleScript_FaintedMonTryChoose
	jumpifword CMP_NO_COMMON_BITS, gHitMarker, HITMARKER_PLAYER_FAINTED, BattleScript_FaintedMonTryChoose
@ Yes/No for sending out a new Pokémon if one is defeated in a wild battle
	printstring STRINGID_USENEXTPKMN
	setbyte gBattleCommunication, 0
	yesnobox
	jumpifbyte CMP_EQUAL, gBattleCommunication + 1, 0, BattleScript_FaintedMonTryChoose
@ Player said no, try to run
	jumpifplayerran BattleScript_FaintedMonEnd
	printstring STRINGID_CANTESCAPE2
BattleScript_FaintedMonTryChoose::
	openpartyscreen BS_FAINTED, BattleScript_FaintedMonEnd
	switchhandleorder BS_FAINTED, 2
	jumpifnotbattletype BATTLE_TYPE_TRAINER, BattleScript_FaintedMonSendOutNew
	jumpifbattletype BATTLE_TYPE_LINK, BattleScript_FaintedMonSendOutNew
	jumpifbattletype BATTLE_TYPE_RECORDED_LINK, BattleScript_FaintedMonSendOutNew
	jumpifbattletype BATTLE_TYPE_FRONTIER, BattleScript_FaintedMonSendOutNew
	jumpifbattletype BATTLE_TYPE_DOUBLE, BattleScript_FaintedMonSendOutNew
	jumpifword CMP_COMMON_BITS, gHitMarker, HITMARKER_PLAYER_FAINTED, BattleScript_FaintedMonSendOutNew
	jumpifbyte CMP_EQUAL, sBATTLE_STYLE, OPTIONS_BATTLE_STYLE_SET, BattleScript_FaintedMonSendOutNew
	jumpifcantswitch BS_PLAYER1, BattleScript_FaintedMonSendOutNew
@ Yes/No for sending out a new Pokémon when the opponent is switching
	printstring STRINGID_ENEMYABOUTTOSWITCHPKMN
	setbyte gBattleCommunication, 0
	yesnobox
	jumpifbyte CMP_EQUAL, gBattleCommunication + 1, 1, BattleScript_FaintedMonSendOutNew
@ Player said yes, go to party screen (note optional flag, player may exit the menu instead)
	setatktoplayer0
	openpartyscreen BS_ATTACKER | PARTY_SCREEN_OPTIONAL, BattleScript_FaintedMonSendOutNew
	switchhandleorder BS_ATTACKER, 2
	jumpifbyte CMP_EQUAL, gBattleCommunication, PARTY_SIZE, BattleScript_FaintedMonSendOutNew
@ Switch Pokémon before opponent
	atknameinbuff1
	resetintimidatetracebits BS_ATTACKER
	hpthresholds2 BS_ATTACKER
	printstring STRINGID_RETURNMON
	switchoutabilities BS_ATTACKER
	waitstate
	returnatktoball
	waitstate
	drawpartystatussummary BS_ATTACKER
	getswitchedmondata BS_ATTACKER
	switchindataupdate BS_ATTACKER
	hpthresholds BS_ATTACKER
	printstring STRINGID_SWITCHINMON
	hidepartystatussummary BS_ATTACKER
	switchinanim BS_ATTACKER, 0
	waitstate
	switchineffects BS_ATTACKER
	resetsentmonsvalue
BattleScript_FaintedMonSendOutNew::
	drawpartystatussummary BS_FAINTED
	getswitchedmondata BS_FAINTED
	switchindataupdate BS_FAINTED
	hpthresholds BS_FAINTED
	printstring STRINGID_SWITCHINMON
	hidepartystatussummary BS_FAINTED
	switchinanim BS_FAINTED, FALSE
	waitstate
	resetplayerfainted
	switchineffects BS_FAINTED
	jumpifbattletype BATTLE_TYPE_DOUBLE, BattleScript_FaintedMonEnd
	cancelallactions
BattleScript_FaintedMonEnd::
	end2

BattleScript_LinkHandleFaintedMonMultiple::
	openpartyscreen BS_FAINTED_LINK_MULTIPLE_1, BattleScript_LinkHandleFaintedMonMultipleStart
BattleScript_LinkHandleFaintedMonMultipleStart::
	switchhandleorder BS_FAINTED, 0
	openpartyscreen BS_FAINTED_LINK_MULTIPLE_2, BattleScript_LinkHandleFaintedMonMultipleEnd
	switchhandleorder BS_FAINTED, 0
BattleScript_LinkHandleFaintedMonLoop::
	switchhandleorder BS_FAINTED, 3
	drawpartystatussummary BS_FAINTED
	getswitchedmondata BS_FAINTED
	switchindataupdate BS_FAINTED
	hpthresholds BS_FAINTED
	printstring STRINGID_SWITCHINMON
	hidepartystatussummary BS_FAINTED
	switchinanim BS_FAINTED, FALSE
	waitstate
	switchineffects BS_FAINTED_LINK_MULTIPLE_1
	jumpifbytenotequal gBattlerFainted, gBattlersCount, BattleScript_LinkHandleFaintedMonLoop
BattleScript_LinkHandleFaintedMonMultipleEnd::
	end2
