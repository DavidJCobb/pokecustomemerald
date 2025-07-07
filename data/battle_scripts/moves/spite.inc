BattleScript_EffectSpite::
	attackcanceler
	ppreduce
	accuracycheck BattleScript_Failed_AtkStringNoPPReduce, ACC_CURR_MOVE
	@
	@ The `tryspiteppreduce` command attempts to reduce the PP of the 
	@ target's last-used move. If it fails, then it jumps to the given 
	@ label. If it succeeeds. it buffers the move name and PP amount 
	@ lost, for printing in battle string messages.
	@
	@ In the vanilla game:
	@
	@  - Attack strings use Buffer 1 on Japanese-only grammar.
	@  - Attack strings use Buffer 2 on the move name.
	@  - Spite uses Buffer 1 for the targeted move.
	@  - Spite uses Buffer 2 for the PP reduction.
	@ 
	@ We've removed the JP grammar code from the STRINGID_USEDMOVE 
	@ handler, freeing up Buffer 1; and we've adjusted Spite to use 
	@ Buffer 3 for the PP reduction.
	@
	tryspiteppreduce BattleScript_Failed_AtkStringNoPPReduce
	
	attackstringandanimation
	waitanimation
	printstring STRINGID_PKMNREDUCEDPP
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
