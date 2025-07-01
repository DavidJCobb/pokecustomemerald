#include "battle_controllers_common.h"
#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_arena.h" // BattleArena_DeductSkillPoints
#include "battle_controllers.h"
#include "battle_gfx_sfx_util.h"
#include "battle_interface.h"
#include "battle_message.h" // BattlePutTextOnWindow, BufferStringBattle
#include "data.h" // gMoveNames
#include "link.h" // GetMultiplayerId
#include "sound.h"
#include "string_util.h" // StringCopy
#include "text.h" // IsTextPrinterActive
#include "util.h" // gBitTable
#include "constants/battle_anim.h"
#include "constants/battle_string_ids.h" // STRINGID_USEDMOVE
#include "constants/sound.h" // CRY_MODE_FAINT

#include "lu/battle_ambient_weather/core.h"

#define CURRENT_LATENT_STATE gBattleStruct->battleControllerLatentState[gActiveBattler]

static u32 ExtractStateDword(u8 index) {
   u16* first = &CURRENT_LATENT_STATE[index];
   return (u32)first[0] | ((u32)first[1] << 16);
}
static void StoreStateDword(u8 index, u32 dword) {
   u16* first = &CURRENT_LATENT_STATE[index];
   first[0] = (u16)dword;
   first[1] = (u16)(dword >> 16);
}

struct Pokemon* GetActiveBattlerPokemonData(void) {
   struct Pokemon* party = 0;
   if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER) {
      party = gPlayerParty;
   } else {
      party = gEnemyParty;
   }
   return &party[gBattlerPartyIndexes[gActiveBattler]];
}

// ---------------------------------------------------------------------------------------------------------------

static void CompleteOnInactiveTextPrinter(void) {
   if (IsTextPrinterActive(B_WIN_MSG))
      return;
   SendControllerCompletionFunc on_complete = (SendControllerCompletionFunc)ExtractStateDword(0);
   on_complete();
}

// ---------------------------------------------------------------------------------------------------------------
//    PUBLIC HELPERS
// ---------------------------------------------------------------------------------------------------------------

extern void BtlController_CommonStartupBehavior(void) {
   //
   // Clear latent state, so we can use it to hold latent task phase enums and 
   // not just callbacks and such.
   //
   memset(CURRENT_LATENT_STATE, 0, sizeof(CURRENT_LATENT_STATE));
}
extern void BtlController_CommonCompletionBehavior(void) {
   if (gBattleTypeFlags & BATTLE_TYPE_LINK) {
      u8 playerId = GetMultiplayerId();

      PrepareBufferDataTransferLink(B_COMM_CONTROLLER_IS_DONE, 4, &playerId);
      gBattleBufferA[gActiveBattler][0] = CONTROLLER_TERMINATOR_NOP;
   } else {
      gBattleControllerExecFlags &= ~gBitTable[gActiveBattler];
   }
   //
   // Clear latent state, so we can use it to hold latent task phase enums and 
   // not just callbacks and such.
   //
   memset(CURRENT_LATENT_STATE, 0, sizeof(CURRENT_LATENT_STATE));
}

// ---------------------------------------------------------------------------------------------------------------
//    MESSAGE HANDLERS WITH LATENT BEHAVIOR
// ---------------------------------------------------------------------------------------------------------------

// Latent handlers for BtlController_HandleReturnMonToBall
static void FreeMonSpriteAfterSwitchOutAnim(void) {
   if (gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].specialAnimActive) {
      return;
   }
   
   FreeSpriteOamMatrix(&gSprites[gBattlerSpriteIds[gActiveBattler]]);
   DestroySprite(&gSprites[gBattlerSpriteIds[gActiveBattler]]);
   if (GetBattlerSide(gActiveBattler) != B_SIDE_PLAYER) {
      //
      // Shadow sprites are only created for non-player-side battlers.
      //
      HideBattlerShadowSprite(gActiveBattler);
   }
   SetHealthboxSpriteInvisible(gHealthboxSpriteIds[gActiveBattler]);
   
   SendControllerCompletionFunc on_complete = (SendControllerCompletionFunc)ExtractStateDword(0);
   on_complete();
}
static void DoSwitchOutAnimation() {
   switch (gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState) {
      case 0:
         if (gBattleSpritesDataPtr->battlerData[gActiveBattler].behindSubstitute) {
            InitAndLaunchSpecialAnimation(gActiveBattler, gActiveBattler, gActiveBattler, B_ANIM_SUBSTITUTE_TO_MON);
         }
         gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 1;
         break;
      case 1:
         if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].specialAnimActive) {
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 0;
            InitAndLaunchSpecialAnimation(
               gActiveBattler,
               gActiveBattler,
               gActiveBattler,
               GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER ? B_ANIM_SWITCH_OUT_PLAYER_MON : B_ANIM_SWITCH_OUT_OPPONENT_MON
            );
            gBattlerControllerFuncs[gActiveBattler] = FreeMonSpriteAfterSwitchOutAnim;
         }
         break;
   }
}
extern void BtlController_HandleReturnMonToBall(SendControllerCompletionFunc on_complete) {
   gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 0;
   StoreStateDword(0, (u32)on_complete);
   if (gBattleBufferA[gActiveBattler][1]) {
      FreeMonSpriteAfterSwitchOutAnim();
      return;
   }
   gBattlerControllerFuncs[gActiveBattler] = DoSwitchOutAnimation;
}

#define lAttackStringState CURRENT_LATENT_STATE[4]
enum {
   ATTACKSTRINGSTATE_UNCHECKED = 0,
   ATTACKSTRINGSTATE_CHECKED,
   ATTACKSTRINGSTATE_PRINTED,
};
// Latent handlers for BtlController_HandleMoveAnimation
static void DoMoveAnimation(void) {
   u16 move     = gBattleBufferA[gActiveBattler][1] | (gBattleBufferA[gActiveBattler][2] << 8);
   u8  multihit = gBattleBufferA[gActiveBattler][11] & 0x7F;

   switch (gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState) {
      case 0:
         if (
            gBattleSpritesDataPtr->battlerData[gActiveBattler].behindSubstitute
            && !gBattleSpritesDataPtr->battlerData[gActiveBattler].flag_x8
         ) {
            gBattleSpritesDataPtr->battlerData[gActiveBattler].flag_x8 = 1;
            InitAndLaunchSpecialAnimation(gActiveBattler, gActiveBattler, gActiveBattler, B_ANIM_SUBSTITUTE_TO_MON);
         }
         gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 1;
         break;
      case 1:
         if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].specialAnimActive) {
            SetBattlerSpriteAffineMode(ST_OAM_AFFINE_OFF);
            DoMoveAnim(move);
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 2;
         }
         break;
      case 2:
         gAnimScriptCallback();
         if (!gAnimScriptActive) {
            SetBattlerSpriteAffineMode(ST_OAM_AFFINE_NORMAL);
            if (gBattleSpritesDataPtr->battlerData[gActiveBattler].behindSubstitute && multihit < 2) {
               InitAndLaunchSpecialAnimation(gActiveBattler, gActiveBattler, gActiveBattler, B_ANIM_MON_TO_SUBSTITUTE);
               gBattleSpritesDataPtr->battlerData[gActiveBattler].flag_x8 = 0;
            }
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 3;
         }
         break;
      case 3:
         if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].specialAnimActive) {
            CopyAllBattleSpritesInvisibilities();
            TrySetBehindSubstituteSpriteBit(gActiveBattler, gBattleBufferA[gActiveBattler][1] | (gBattleBufferA[gActiveBattler][2] << 8));
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 0;
            if (lAttackStringState == ATTACKSTRINGSTATE_PRINTED) {
               gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 4;
            } else {
               SendControllerCompletionFunc on_complete = (SendControllerCompletionFunc)ExtractStateDword(0);
               on_complete();
            }
         }
         break;
      case 4:
         CompleteOnInactiveTextPrinter();
         break;
   }
}
extern bool BtlController_HandleMoveAnimation(SendControllerCompletionFunc on_complete) {
   if (lAttackStringState == ATTACKSTRINGSTATE_UNCHECKED) {
      lAttackStringState = ATTACKSTRINGSTATE_CHECKED;
      bool wants_attack_string = (gBattleBufferA[gActiveBattler][11] >> 7) != 0;
      if (wants_attack_string) {
         gBattle_BG0_X = 0;
         gBattle_BG0_Y = 0;
         
         u16 move = gBattleBufferA[gActiveBattler][1] | (gBattleBufferA[gActiveBattler][2] << 8);
         struct BattleMsgData data = {
            .currentMove = move,
            .moveType    = TYPE_MYSTERY,
         };
         BufferStringBattleWithData(STRINGID_USEDMOVE, &data);
         BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
         BattleArena_DeductSkillPoints(gActiveBattler, STRINGID_USEDMOVE);
         
         lAttackStringState = ATTACKSTRINGSTATE_PRINTED;
      }
   }
   
   if (!IsBattleSEPlaying(gActiveBattler) && !IsBattleAmbientWeatherPlaying()) {
      u16 move = gBattleBufferA[gActiveBattler][1] | (gBattleBufferA[gActiveBattler][2] << 8);

      gAnimMoveTurn         = gBattleBufferA[gActiveBattler][3];
      gAnimMovePower        = gBattleBufferA[gActiveBattler][4] | (gBattleBufferA[gActiveBattler][5] << 8);
      gAnimMoveDmg          = gBattleBufferA[gActiveBattler][6] | (gBattleBufferA[gActiveBattler][7] << 8) | (gBattleBufferA[gActiveBattler][8] << 16) | (gBattleBufferA[gActiveBattler][9] << 24);
      gAnimFriendship       = gBattleBufferA[gActiveBattler][10];
      gWeatherMoveAnim      = gBattleBufferA[gActiveBattler][12] | (gBattleBufferA[gActiveBattler][13] << 8);
      gAnimDisableStructPtr = (struct DisableStruct *)&gBattleBufferA[gActiveBattler][16];
      gTransformedPersonalities[gActiveBattler] = gAnimDisableStructPtr->transformedMonPersonality;
      if (IsMoveWithoutAnimation(move, gAnimMoveTurn)) {
         on_complete();
         return false;
      }
      
      StoreStateDword(0, (u32)on_complete);
      gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 0;
      gBattlerControllerFuncs[gActiveBattler] = DoMoveAnimation;
      return true;
   }
   return false;
}
#undef lAttackStringState

extern void BtlController_HandlePrintString(SendControllerCompletionFunc on_complete) {
   u16* stringId = (u16*)(&gBattleBufferA[gActiveBattler][2]);
   
   gBattle_BG0_X = 0;
   gBattle_BG0_Y = 0;
   BufferStringBattle(*stringId);
   BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
   
   StoreStateDword(0, (u32)on_complete);
   gBattlerControllerFuncs[gActiveBattler] = CompleteOnInactiveTextPrinter;
   
   BattleArena_DeductSkillPoints(gActiveBattler, *stringId);
}

static void CompleteOnHealthbarDone(void) {
   s16 hpValue = MoveBattleBar(gActiveBattler, gHealthboxSpriteIds[gActiveBattler], HEALTH_BAR, 0);
   SetHealthboxSpriteVisible(gHealthboxSpriteIds[gActiveBattler]);
   if (hpValue != -1) {
      UpdateHpTextInHealthbox(gHealthboxSpriteIds[gActiveBattler], hpValue, HP_CURRENT);
   } else {
      SendControllerCompletionFunc on_complete = (SendControllerCompletionFunc)ExtractStateDword(0);
      on_complete();
   }
}
extern s16 BtlController_HandleHealthBarUpdate(SendControllerCompletionFunc on_complete) {
   s16 hpVal = (gBattleBufferA[gActiveBattler][3] << 8) | gBattleBufferA[gActiveBattler][2];

   LoadBattleBarGfx(0);

   struct Pokemon* pokemon = GetActiveBattlerPokemonData();
   u32 maxHP = GetMonData(pokemon, MON_DATA_MAX_HP);
   if (hpVal != INSTANT_HP_BAR_DROP) {
      u32 curHP = GetMonData(pokemon, MON_DATA_HP);
      SetBattleBarStruct(gActiveBattler, gHealthboxSpriteIds[gActiveBattler], maxHP, curHP, hpVal);
   } else {
      SetBattleBarStruct(gActiveBattler, gHealthboxSpriteIds[gActiveBattler], maxHP, 0, hpVal);
   }

   StoreStateDword(0, (u32)on_complete);
   gBattlerControllerFuncs[gActiveBattler] = CompleteOnHealthbarDone;
   
   return hpVal;
}

static void CompleteOnFinishedStatusAnimation(void) {
   if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].statusAnimActive) {
      SendControllerCompletionFunc on_complete = (SendControllerCompletionFunc)ExtractStateDword(0);
      on_complete();
   }
}
extern void BtlController_HandleStatusIconUpdate(SendControllerCompletionFunc on_complete) {
   if (IsBattleSEPlaying(gActiveBattler))
      return;
   
   UpdateHealthboxAttribute(gHealthboxSpriteIds[gActiveBattler], GetActiveBattlerPokemonData(), HEALTHBOX_STATUS_ICON);
   gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].statusAnimActive = 0;
   
   StoreStateDword(0, (u32)on_complete);
   gBattlerControllerFuncs[gActiveBattler] = CompleteOnFinishedStatusAnimation;
}
extern void BtlController_HandleStatusAnimation(SendControllerCompletionFunc on_complete) {
   if (IsBattleSEPlaying(gActiveBattler))
      return;
   
   InitAndLaunchChosenStatusAnimation(
      gBattleBufferA[gActiveBattler][1],
      gBattleBufferA[gActiveBattler][2] | (gBattleBufferA[gActiveBattler][3] << 8) | (gBattleBufferA[gActiveBattler][4] << 16) | (gBattleBufferA[gActiveBattler][5] << 24)
   );
   
   StoreStateDword(0, (u32)on_complete);
   gBattlerControllerFuncs[gActiveBattler] = CompleteOnFinishedStatusAnimation;
}

static void CompleteOnFinishedBattleAnimation(void) {
   if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animFromTableActive) {
      SendControllerCompletionFunc on_complete = (SendControllerCompletionFunc)ExtractStateDword(0);
      on_complete();
   }
}
extern bool BtlController_HandleBattleAnimation(SendControllerCompletionFunc on_complete, bool wait_for_sounds) {
   if (!wait_for_sounds || !IsBattleSEPlaying(gActiveBattler)) {
      u8  animationId = gBattleBufferA[gActiveBattler][1];
      u16 argument    = gBattleBufferA[gActiveBattler][2] | (gBattleBufferA[gActiveBattler][3] << 8);

      if (TryHandleLaunchBattleTableAnimation(gActiveBattler, gActiveBattler, gActiveBattler, animationId, argument)) {
         on_complete();
      } else {
         StoreStateDword(0, (u32)on_complete);
         gBattlerControllerFuncs[gActiveBattler] = CompleteOnFinishedBattleAnimation;
      }
      return true;
   }
   return false;
}

// sprite data:
#define sCounter data[1]
static void DoHitAnimBlinkSpriteEffect(void) {
   u8 spriteId = gBattlerSpriteIds[gActiveBattler];

   if (gSprites[spriteId].sCounter == 32) {
      gSprites[spriteId].sCounter  = 0;
      gSprites[spriteId].invisible = FALSE;
      gDoingBattleAnim = FALSE;
      
      SendControllerCompletionFunc on_complete = (SendControllerCompletionFunc)ExtractStateDword(0);
      on_complete();
   } else {
      if ((gSprites[spriteId].sCounter % 4) == 0)
         gSprites[spriteId].invisible ^= 1;
      gSprites[spriteId].sCounter++;
   }
}
extern void BtlController_HandleHitAnimation(SendControllerCompletionFunc on_complete) {
   if (gSprites[gBattlerSpriteIds[gActiveBattler]].invisible == TRUE) {
      on_complete();
      return;
   }
   
   gDoingBattleAnim = TRUE;
   gSprites[gBattlerSpriteIds[gActiveBattler]].sCounter = 0;
   DoHitAnimHealthboxEffect(gActiveBattler);
   
   StoreStateDword(0, (u32)on_complete);
   gBattlerControllerFuncs[gActiveBattler] = DoHitAnimBlinkSpriteEffect;
}
#undef sCounter

// ---------------------------------------------------------------------------------------------------------------
//    MESSAGE HANDLERS WITH INSTANT BEHAVIOR
// ---------------------------------------------------------------------------------------------------------------

extern void BtlController_HandlePlaySE(void) {
   s8 pan;

   // Despite the naming of these constants, they don't vary based on what 
   // side of the battlefield the active battler is on. That is: the player 
   // and opponent controllers both use SOUND_PAN_ATTACKER for player-side 
   // battlers, and SOUND_PAN_TARGET for enemy-side battlers.
   if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
      pan = SOUND_PAN_ATTACKER;
   else
      pan = SOUND_PAN_TARGET;

   PlaySE12WithPanning(gBattleBufferA[gActiveBattler][1] | (gBattleBufferA[gActiveBattler][2] << 8), pan);
}

extern void BtlController_HandlePlayFanfareOrBGM(void) {
   if (gBattleBufferA[gActiveBattler][3])  {
      BattleStopLowHpSound();
      PlayBGM(gBattleBufferA[gActiveBattler][1] | (gBattleBufferA[gActiveBattler][2] << 8));
   } else {
      PlayFanfare(gBattleBufferA[gActiveBattler][1] | (gBattleBufferA[gActiveBattler][2] << 8));
   }
}

extern void BtlController_HandleFaintingCry(void) {
   u16 species = GetMonData(GetActiveBattlerPokemonData(), MON_DATA_SPECIES);

   PlayCry_ByMode(
      species,
      (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER) ? -25 : 25,
      CRY_MODE_FAINT
   );
}

extern void BtlController_HandleIntroSlide(void) {
   HandleIntroSlide(gBattleBufferA[gActiveBattler][1]);
   gIntroSlideFlags |= 1;
}

extern void BtlController_HandleSpriteInvisibility(void) {
   if (IsBattlerSpritePresent(gActiveBattler)) {
      gSprites[gBattlerSpriteIds[gActiveBattler]].invisible = gBattleBufferA[gActiveBattler][1];
      CopyBattleSpriteInvisibility(gActiveBattler);
   }
}

