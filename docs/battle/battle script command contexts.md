
# Battle script command contexts

## `critmessage`

Preconditions:

* `gBattlerAttacker` must have been set up
* `gCritMultiplier` must have been set up
* `gMoveResultFlags` must have been set up (we check `MOVE_RESULT_NO_EFFECT`)

## `resultmessage`

Preconditions:

* `gBattlerAttacker` must have been set up
* `gBattlerTarget` must have been set up
* `gMoveResultFlags` must have been set up
* `gBattleCommunication[MISS_TYPE]` must have been set up

