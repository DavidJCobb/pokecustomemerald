
@
@ Using Defense Curl sets a STATUS2 flag which buffs the power of Rollout.
@

BattleScript_EffectDefenseCurl::
	attackcanceler
	ppreduce
	setdefensecurlbit
	trystatchange (BIT_DEF), (MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_DEFER_ALL_VISUALS), 1, STAT_CHANGE_CAUSE_MOVE_EFFECT_PRIMARY, BattleScript_EffectDefenseCurl_Failed
	attackstringandanimation
	waitanimation
BattleScript_EffectDefenseCurl_End::
	showstatchange
	goto BattleScript_MoveEnd

BattleScript_EffectDefenseCurl_Failed::
	attackstring
	goto BattleScript_EffectDefenseCurl_End
