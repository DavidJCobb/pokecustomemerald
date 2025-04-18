EventScript_RepelWoreOff::
	goto_if_eq VAR_REPEL_ITEM_TYPE, 0, EventScript_RepelWoreOff_Vague
   @
   @ We know the repel type that the player last used.
   @
	bufferitemname STR_VAR_1, VAR_REPEL_ITEM_TYPE
	lu_getitemcount VAR_REPEL_ITEM_TYPE
	goto_if_eq VAR_RESULT, 0, EventScript_RepelWoreOffDetailed_NoMore
	buffernumberstring STR_VAR_2, VAR_RESULT
   @
   loadword 0, Text_RepelWoreOff_SpecificPromptUse
   goto_if_lt VAR_RESULT, 1000, EventScript_RepelWoreOff_AskToUseAnother
   loadword 0, Text_RepelWoreOff_SpecificPromptUseMany
EventScript_RepelWoreOff_AskToUseAnother:
   lockall
	message NULL
	waitmessage
	yesnobox 20, 8
	goto_if_eq VAR_RESULT, 0, EventScript_RepelWoreOff_Exit
	special UseAnotherRepel
EventScript_RepelWoreOff_Exit:
   releaseall
	end

EventScript_RepelWoreOff_Vague:
	msgbox Text_RepelWoreOff, MSGBOX_SIGN
	end
   
EventScript_RepelWoreOffDetailed_NoMore:
	msgbox Text_RepelWoreOff_SpecificNoneRemain, MSGBOX_SIGN
	end

Text_RepelWoreOff:
	.string "REPEL's effect wore off…$"

Text_RepelWoreOff_SpecificNoneRemain:
	.string "{STR_VAR_1}'s effect wore off.\n$"
   .string "You don't have any more left…$"

Text_RepelWoreOff_SpecificPromptUse:
	.string "{STR_VAR_1}'s effect wore off.\n"
	.string "You have {STR_VAR_2} more. Use another?$"

Text_RepelWoreOff_SpecificPromptUseMany:
	.string "{STR_VAR_1}'s effect wore off.\n"
	.string "You have a ton more. Use another?$"
