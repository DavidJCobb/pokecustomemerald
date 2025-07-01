BattleScript_ActionSwitch::
	hpthresholds2 BS_ATTACKER
	printstring STRINGID_RETURNMON
	setbyte sDMG_MULTIPLIER, 2
	jumpifbattletype BATTLE_TYPE_DOUBLE, BattleScript_PursuitSwitchDmgSetMultihit
	setmultihit 1
	goto BattleScript_PursuitSwitchDmgLoop
BattleScript_PursuitSwitchDmgSetMultihit::
	setmultihit 2
BattleScript_PursuitSwitchDmgLoop::
	jumpifnopursuitswitchdmg BattleScript_DoSwitchOut
	swapattackerwithtarget
	trysetdestinybondtohappen
	call BattleScript_PursuitDmgOnSwitchOut
	swapattackerwithtarget
BattleScript_DoSwitchOut::
	decrementmultihit BattleScript_PursuitSwitchDmgLoop
	switchoutabilities BS_ATTACKER
	waitstate
	returnatktoball
	waitstate
	drawpartystatussummary BS_ATTACKER
	switchhandleorder BS_ATTACKER, 1
	getswitchedmondata BS_ATTACKER
	switchindataupdate BS_ATTACKER
	hpthresholds BS_ATTACKER
	printstring STRINGID_SWITCHINMON
	hidepartystatussummary BS_ATTACKER
	switchinanim BS_ATTACKER, FALSE
	waitstate
	switchineffects BS_ATTACKER
	moveendcase MOVEEND_IMMUNITY_ABILITIES
	moveendcase MOVEEND_MIRROR_MOVE
	end2

BattleScript_PursuitDmgOnSwitchOut::
	pause B_WAIT_TIME_SHORT
	attackstring
	ppreduce
	critcalc
	damagecalc
	typecalc
	adjustnormaldamage
	attackanimation
	waitanimation
	effectivenesssound
	hitanimation BS_TARGET
	waitstate
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	critmessage
	waitmessage B_WAIT_TIME_LONG
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	tryfaintmon BS_TARGET
	moveendfromto MOVEEND_ON_DAMAGE_ABILITIES, MOVEEND_CHOICE_MOVE
	getbattlerfainted BS_TARGET
	jumpifbyte CMP_EQUAL, gBattleCommunication, FALSE, BattleScript_PursuitDmgOnSwitchOutRet
	setbyte sGIVEEXP_STATE, 0
	getexp BS_TARGET
BattleScript_PursuitDmgOnSwitchOutRet:
	return
