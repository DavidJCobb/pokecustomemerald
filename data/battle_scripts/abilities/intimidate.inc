BattleScript_IntimidateActivatesEnd3::
	call BattleScript_PauseIntimidateActivates
	end3

BattleScript_PauseIntimidateActivates:
	pause B_WAIT_TIME_SHORT
BattleScript_IntimidateActivates::
	setbyte gBattlerTarget, 0
	setstatchanger STAT_ATK, 1, TRUE
BattleScript_IntimidateActivatesLoop:

   @ If `gBattlerTarget` does not already refer to a valid target 
   @ for Intimidate (i.e. a non-absent battler on the opposite side 
   @ of the field), then this script command increases it until it 
   @ either finds a valid target, or runs out of battlers to check. 
   @ If the latter outcome occurs, the command jumps to the given 
   @ label.
   @
	trygetintimidatetarget BattleScript_IntimidateActivatesReturn
   
	jumpifstatus2 BS_TARGET, STATUS2_SUBSTITUTE, BattleScript_IntimidateActivatesLoopIncrement
   
   trystatchange (BIT_ATK), (STAT_CHANGE_NOT_PROTECT_AFFECTED), -1, STAT_CHANGE_CAUSE_ABILITY, 0
   
BattleScript_IntimidateActivatesLoopIncrement:
	addbyte gBattlerTarget, 1
	goto BattleScript_IntimidateActivatesLoop
BattleScript_IntimidateActivatesReturn:
	return