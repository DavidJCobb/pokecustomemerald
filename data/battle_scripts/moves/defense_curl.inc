
@
@ Using Defense Curl sets a STATUS2 flag which buffs the power of Rollout.
@

BattleScript_EffectDefenseCurl::
	attackcanceler
	ppreduce
	setdefensecurlbit
	setstatchanger STAT_DEF, 1, FALSE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_DefenseCurlDoStatUpAnim
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_STAT_WONT_INCREASE, BattleScript_EffectDefenseCurl_Failed
	attackstringandanimation
	waitanimation
BattleScript_DefenseCurlDoStatUpAnim::
	attackstring
	goto BattleScript_StatUpDoAnim

BattleScript_EffectDefenseCurl_Failed::
	attackstring
	goto BattleScript_StatUpPrintString