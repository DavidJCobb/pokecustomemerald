BattleScript_EffectLeechSeed::
	attackcanceler
	pause B_WAIT_TIME_SHORT
	ppreduce
	jumpifstatus2 BS_TARGET, STATUS2_SUBSTITUTE, BattleScript_Failed_AtkStringNoPPReduce
	accuracycheck BattleScript_DoLeechSeed, ACC_CURR_MOVE
	@
	@ That `accuracycheck` command might seem redundant, since it doesn't jump out 
	@ of this script if the attack "misses." However, the `setseeded` command will 
   @ vary its behavior based on whether the move result flags include "missed" 
   @ (and `accuracycheck` contains a hardcoded conditional jump to scripts for 
   @ Protect, as well). The `attackanimation` and `attackstringandanimation` 
   @ commands check the same result flag.
	@
BattleScript_DoLeechSeed::
	setseeded
	attackstringandanimation
	waitanimation
	printfromtable gLeechSeedStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_LeechSeedTurnDrain::
	playanimation BS_ATTACKER, B_ANIM_LEECH_SEED_DRAIN, sB_ANIM_ARG1
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_DAMAGE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	copyword gBattleMoveDamage, gHpDealt
	jumpifability BS_ATTACKER, ABILITY_LIQUID_OOZE, BattleScript_LeechSeedTurnPrintLiquidOoze
	manipulatedamage DMG_CHANGE_SIGN
	setbyte cMULTISTRING_CHOOSER, B_MSG_LEECH_SEED_DRAIN
	goto BattleScript_LeechSeedTurnPrintAndUpdateHp
BattleScript_LeechSeedTurnPrintLiquidOoze::
	setbyte cMULTISTRING_CHOOSER, B_MSG_LEECH_SEED_OOZE
BattleScript_LeechSeedTurnPrintAndUpdateHp::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_DAMAGE
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	printfromtable gLeechSeedStringIds
	waitmessage B_WAIT_TIME_LONG
	tryfaintmon BS_ATTACKER
	tryfaintmon BS_TARGET
	end2

BattleScript_LeechSeedFree::
	printstring STRINGID_PKMNSHEDLEECHSEED
	waitmessage B_WAIT_TIME_LONG
	return
