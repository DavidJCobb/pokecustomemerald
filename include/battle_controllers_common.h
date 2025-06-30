#ifndef GUARD_LU_BATTLE_CONTROLLER_COMMON
#define GUARD_LU_BATTLE_CONTROLLER_COMMON

typedef void(*)() SendControllerCompletionFunc;

extern void BtlController_HandleReturnMonToBall(SendControllerCompletionFunc);

// Returns true if an animation begins playing. Some controllers, such 
// as the player, do some extra bookkeeping in that instance. Returns 
// false if the animation either has not yet started playing (e.g. 
// because we're waiting on something) or has been skipped.
extern bool BtlController_HandleMoveAnimation(SendControllerCompletionFunc);

extern void BtlController_HandlePrintString(SendControllerCompletionFunc);

// Returns the HP value.
extern s16 BtlController_HandleHealthBarUpdate(SendControllerCompletionFunc);

extern void BtlController_HandleStatusIconUpdate(SendControllerCompletionFunc);
extern void BtlController_HandleStatusAnimation(SendControllerCompletionFunc);

// Returns true if an animation begins playing. Some controllers, such 
// as the player, do some extra bookkeeping in that instance. Returns 
// false if the animation either has not yet started playing (e.g. 
// because we're waiting on something) or has been skipped.
extern bool BtlController_HandleBattleAnimation(SendControllerCompletionFunc, bool wait_for_sounds);

extern void BtlController_HandleHitAnimation(SendControllerCompletionFunc);

// -----------------------------------------------------------------------------

// These handlers aren't latent, so there's no need to take the completion 
// function as a parameter. The controller should just invoke this, and then 
// invoke the completion function itself right afterward.
extern void BtlController_HandlePlaySE(void);
extern void BtlController_HandlePlayFanfareOrBGM(void);
extern void BtlController_HandleFaintingCry(void);
extern void BtlController_HandleIntroSlide(void);

extern void BtlController_HandleSpriteInvisibility(void);

#endif