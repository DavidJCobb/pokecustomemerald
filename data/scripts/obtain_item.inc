.set ITEMID, VAR_0x8000
.set AMOUNT, VAR_0x8001
.set SUCCESSFULLY_ADDED_ITEM, VAR_0x8007

Std_ObtainItem::
	additem ITEMID, AMOUNT
	copyvar SUCCESSFULLY_ADDED_ITEM, VAR_RESULT
	call EventScript_ObtainItemMessage
	return

EventScript_ObtainItemMessage::
	bufferitemnameplural STR_VAR_2, ITEMID, AMOUNT
	checkitemtype ITEMID
	call EventScript_BufferPocketNameAndTryFanfare
	call_if_eq SUCCESSFULLY_ADDED_ITEM, TRUE, EventScript_ObtainedItem
	call_if_eq SUCCESSFULLY_ADDED_ITEM, FALSE, EventScript_NoRoomForItem
	return

EventScript_BufferPocketNameAndTryFanfare::
	switch VAR_RESULT
	case POCKET_ITEMS,		EventScript_BufferItemsPocket
	case POCKET_KEY_ITEMS,  EventScript_BufferKeyItemsPocket
	case POCKET_POKE_BALLS, EventScript_BufferPokeballsPocket
	case POCKET_TM_HM,		EventScript_BufferTMHMsPocket
	case POCKET_BERRIES,	 EventScript_BufferBerriesPocket
	end

EventScript_BufferItemsPocket::
	bufferstdstring STR_VAR_3, STDSTRING_ITEMS
	call_if_eq SUCCESSFULLY_ADDED_ITEM, TRUE, EventScript_PlayFanfareObtainedItem
	return

EventScript_BufferKeyItemsPocket::
	bufferstdstring STR_VAR_3, STDSTRING_KEYITEMS
	call_if_eq SUCCESSFULLY_ADDED_ITEM, TRUE, EventScript_PlayFanfareObtainedItem
	return

EventScript_BufferPokeballsPocket::
	bufferstdstring STR_VAR_3, STDSTRING_POKEBALLS
	call_if_eq SUCCESSFULLY_ADDED_ITEM, TRUE, EventScript_PlayFanfareObtainedItem
	return

EventScript_BufferTMHMsPocket::
	bufferstdstring STR_VAR_3, STDSTRING_TMHMS
	call_if_eq SUCCESSFULLY_ADDED_ITEM, TRUE, EventScript_PlayFanfareObtainedTMHM
	return

EventScript_BufferBerriesPocket::
	bufferstdstring STR_VAR_3, STDSTRING_BERRIES
	call_if_eq SUCCESSFULLY_ADDED_ITEM, TRUE, EventScript_PlayFanfareObtainedItem
	return

EventScript_ObtainedItem::
	message gText_ObtainedTheItem
	waitfanfare
	msgbox gText_PutItemInPocket, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

EventScript_NoRoomForItem::
	setvar VAR_RESULT, FALSE
	return

EventScript_PlayFanfareObtainedItem::
	playfanfare MUS_OBTAIN_ITEM
	return

EventScript_PlayFanfareObtainedTMHM::
	playfanfare MUS_OBTAIN_TMHM
	return

@@@@@@@@ OBTAINING A DECORATION @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

Std_ObtainDecoration::
	adddecoration ITEMID
	copyvar SUCCESSFULLY_ADDED_ITEM, VAR_RESULT
	call EventScript_ObtainDecorationMessage
	return

EventScript_ObtainDecorationMessage::
	bufferdecorationname STR_VAR_2, ITEMID
	call_if_eq SUCCESSFULLY_ADDED_ITEM, TRUE, EventScript_ObtainedDecor
	call_if_eq SUCCESSFULLY_ADDED_ITEM, FALSE, EventScript_NoRoomForDecor
	return

EventScript_ObtainedDecor::
	playfanfare MUS_OBTAIN_ITEM
	message gText_ObtainedTheDecor
	waitfanfare
	msgbox gText_TheDecorWasTransferredToThePC, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

EventScript_NoRoomForDecor::
	setvar VAR_RESULT, FALSE
	return

@@@@@@@@ FINDING AN ITEM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@ Std_FindItem
@  - EventScript_BufferPocketNameAndTryFanfare
@  = branch
@	  - EventScript_PickUpItem
@		  = branch
@			  - EventScript_FoundTMHM
@			  - EventScript_FoundItem
@	  - EventScript_NoRoomToPickUpItem
@

Std_FindItem::
	lock
	faceplayer
	waitse
	copyvar VAR_0x8004, ITEMID @ argument to checkitemspace
	copyvar VAR_0x8005, AMOUNT @ argument to checkitemspace
	checkitemspace ITEMID, AMOUNT
	copyvar SUCCESSFULLY_ADDED_ITEM, VAR_RESULT
	bufferitemnameplural STR_VAR_2, ITEMID, AMOUNT
	checkitemtype ITEMID
	call EventScript_BufferPocketNameAndTryFanfare
	call_if_eq SUCCESSFULLY_ADDED_ITEM, TRUE, EventScript_PickUpItem
	call_if_eq SUCCESSFULLY_ADDED_ITEM, FALSE, EventScript_NoRoomToPickUpItem
	release
	return

EventScript_PickUpItem::
	removeobject VAR_LAST_TALKED
	additem VAR_0x8004, VAR_0x8005	
	@
	@ Try to buffer the item's TM/HM move name. If it works, then 
	@ the name will be written to STR_VAR_2 and VAR_RESULT will be 
	@ set to 1. Otherwise, the item isn't a TM, and STR_VAR_2 will 
	@ be unchanged from its previous value (the pluralized item 
	@ name).
	@
	specialvar VAR_RESULT, BufferTMHMMoveName
	copyvar VAR_0x8008, VAR_RESULT
	call_if_eq VAR_0x8008, TRUE, EventScript_FoundTMHM
	call_if_eq VAR_0x8008, FALSE, EventScript_FoundItem
	waitfanfare
	waitmessage
	waitbuttonpress
	return

EventScript_FoundTMHM::
	bufferitemnameplural STR_VAR_1, VAR_0x8004, VAR_0x8005
	@
	@ STR_VAR_2: the move name
	@ STR_VAR_3: the bag pocket name
	@
	loadword 0, gText_lu_FoundAndPocketedOneTMHM
	pyramid_inchallenge
	call_if_eq VAR_RESULT, TRUE, EventScript_FoundTMHM_PrepTextIfBattlePyramid
	lu_message_wordwrapped 0
	return

EventScript_FoundTMHM_PrepTextIfBattlePyramid::
	loadword 0, gText_lu_FoundAndBaggedOneTMHM
	return

EventScript_FoundItem::
	loadword 0, gText_lu_FoundAndPocketedOneItem
	pyramid_inchallenge
	call_if_eq VAR_RESULT, TRUE, EventScript_FoundItem_PrepTextIfBattlePyramid
	lu_message_wordwrapped 0
	return

EventScript_FoundItem_PrepTextIfBattlePyramid::
	loadword 0, gText_lu_FoundAndBaggedOneItem
	return

EventScript_NoRoomToPickUpItem::
	lu_msgbox_default_wordwrapped gText_lu_FoundOneItemButBagFull
	setvar VAR_RESULT, FALSE
	return

@@@@@@@@ FINDING A HIDDEN ITEM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.set HIDDENITEM_FLAG,    VAR_0x8004 @ Have we found this hidden item yet?
.set HIDDENITEM_TYPE,    VAR_0x8005 @ What ItemIDGlobal is this?
.set HIDDENITEM_IS_TMHM, VAR_0x8008

EventScript_HiddenItemScript::
	lockall
	waitse
	additem HIDDENITEM_TYPE
	copyvar SUCCESSFULLY_ADDED_ITEM, VAR_RESULT
	bufferitemnameplural STR_VAR_2, HIDDENITEM_TYPE, 1
   goto_if_eq SUCCESSFULLY_ADDED_ITEM, FALSE, EventScript_NoRoomForHiddenItem
	checkitemtype HIDDENITEM_TYPE
	call EventScript_BufferPocketNameAndTryFanfare
   @
	copyvar HIDDENITEM_IS_TMHM, HIDDENITEM_FLAG    @ protect variable
	copyvar HIDDENITEM_FLAG,    HIDDENITEM_TYPE    @ argument to BufferTMHMMoveName
	specialvar VAR_RESULT, BufferTMHMMoveName
	copyvar HIDDENITEM_FLAG,    HIDDENITEM_IS_TMHM @ restore variable
	copyvar HIDDENITEM_IS_TMHM, VAR_RESULT
   @
	call_if_eq HIDDENITEM_IS_TMHM, TRUE, EventScript_FoundHiddenTMHM
	call_if_eq HIDDENITEM_IS_TMHM, FALSE, EventScript_FoundHiddenItem
	goto EventScript_PutHiddenItemInPocket

EventScript_FoundHiddenTMHM::
	bufferitemnameplural STR_VAR_1, HIDDENITEM_TYPE, 1
	lu_message_wordwrapped gText_lu_FoundAndPocketedOneTMHM
	return

EventScript_FoundHiddenItem::
	lu_message_wordwrapped gText_lu_FoundAndPocketedOneItem
	return

EventScript_PutHiddenItemInPocket::
	waitfanfare
	waitmessage
	waitbuttonpress
	special TryPutTreasureInvestigatorsOnAir
	special SetHiddenItemFlag @ Reads HIDDENITEM_FLAG as its argument.
	releaseall
	end

EventScript_NoRoomForHiddenItem::
	lu_msgbox_default_wordwrapped gText_lu_FoundOneItemButBagFull
	setvar VAR_RESULT, 0
	releaseall
	end
