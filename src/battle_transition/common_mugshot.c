#include "battle_transition/common.h"
#include "battle_transition/common_mugshot.h"
#include "battle_transition/clockwise_wipe.h"
#include "gba/gba.h"
#include "decompress.h"
#include "gpu_regs.h"
#include "constants/global.h" // GENDER_COUNT
#include "constants/rgb.h"
#include "constants/songs.h" // SE_MUGSHOT
#include "field_effect.h" // CreateTrainerSprite
#include "main.h" // SetHBlankCallback, SetVBlankCallback
#include "palette.h"
#include "scanline_effect.h" // gScanlineEffectRegBuffers
#include "sound.h" // PlaySE
#include "sprite.h"
#include "task.h"
#include "trig.h"

#include "global.h" // gSaveBlock2Ptr->playerGender

#define SET_TILE(ptr, posY, posX, tile) \
{                                       \
    u32 index = (posY) * 32 + posX;     \
    ptr[index] = tile | (0xF0 << 8);    \
}

#define tSinIndex           data[1]
#define tTopBannerX         data[2]
#define tBottomBannerX      data[3]
#define tTimer              data[3] // Re-used
#define tFadeSpread         data[4]
#define tOpponentSpriteId   data[13]
#define tPlayerSpriteId     data[14]
#define tMugshotId          data[15]

// Sprite data for trainer sprites in mugshots
#define sState       data[0]
#define sSlideSpeed  data[1]
#define sSlideAccel  data[2]
#define sDone        data[6]
#define sSlideDir    data[7]

enum TASK_STATE {
   TASK_STATE_INIT,
   TASK_STATE_SET_GFX,
   TASK_STATE_SHOW_BANNER,
   TASK_STATE_START_OPPONENT_SLIDE,
   TASK_STATE_WAIT_BETWEEN_SLIDES,
   TASK_STATE_WAIT_PLAYER_SLIDE,
   TASK_STATE_GRADUAL_WHITE_FADE,
   TASK_STATE_INIT_FADE_WHITE_TO_BLACK,
   TASK_STATE_FADE_TO_BLACK,
   TASK_STATE_END,
};
static void HBlankCB_Mugshots(void);
static void VBlankCB_Mugshots(void);
static void VBlankCB_MugshotsFadeOut(void);
static void Mugshots_CreateTrainerPics(struct Task*, const struct BattleTransitionMugshot*);
static void SpriteCB_MugshotTrainerPic(struct Sprite*);
static void SetTrainerPicSlideDirection(s16 spriteId, s16 dirId);
static void IncrementTrainerPicState(s16 spriteId);
static s16  IsTrainerPicSlideDone(s16 spriteId);

static const u16 sMugshotsTilemap[]   = INCBIN_U16("graphics/battle_transitions/elite_four_bg_map.bin");
static const u32 sEliteFour_Tileset[] = INCBIN_U32("graphics/battle_transitions/elite_four_bg.4bpp");

static const u16 sMugshotPal_Brendan[] = INCBIN_U16("graphics/battle_transitions/brendan_bg.gbapal");
static const u16 sMugshotPal_May[]     = INCBIN_U16("graphics/battle_transitions/may_bg.gbapal");
static const u16 *const sPlayerMugshotsPals[GENDER_COUNT] = {
   [MALE]   = sMugshotPal_Brendan,
   [FEMALE] = sMugshotPal_May
};

extern void BattleTransitionCommon_Mugshot_Exec(
   struct Task* task,
   const struct BattleTransitionMugshot* params
) {
   switch (task->tState) {
      case TASK_STATE_INIT:
         BattleTransitionCommon_InitTransitionData();
         ScanlineEffect_Clear();
         Mugshots_CreateTrainerPics(task, params);

         task->tSinIndex      = 0;
         task->tTopBannerX    = 1;
         task->tBottomBannerX = DISPLAY_WIDTH - 1;
         gBattleTransitionData->WININ  = WININ_WIN0_ALL;
         gBattleTransitionData->WINOUT = WINOUT_WIN01_BG1 | WINOUT_WIN01_BG2 | WINOUT_WIN01_BG3 | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR;
         gBattleTransitionData->WIN0V  = DISPLAY_HEIGHT;

         for (u8 i = 0; i < DISPLAY_HEIGHT; i++)
           gScanlineEffectRegBuffers[1][i] = (DISPLAY_WIDTH << 8) | (DISPLAY_WIDTH + 1);

         SetVBlankCallback(VBlankCB_Mugshots);

         task->tState++;
         break;
      case TASK_STATE_SET_GFX:
         {
            u16* tilemap;
            u16* tileset;
            BattleTransitionCommon_GetBg0TilesDst(&tilemap, &tileset);
            
            CpuSet(sEliteFour_Tileset, tileset, 0xF0);
            
            LoadPalette(params->opponent.banner_palette.data, BG_PLTT_ID(15), params->opponent.banner_palette.size);
            LoadPalette(sPlayerMugshotsPals[gSaveBlock2Ptr->playerGender], BG_PLTT_ID(15) + 10, PLTT_SIZEOF(6));
            
            const u16* src = sMugshotsTilemap;
            for (s16 i = 0; i < 20; i++) {
               for (s16 j = 0; j < 32; j++, src++)
                  SET_TILE(tilemap, i, j, *src);
            }
         }
         EnableInterrupts(INTR_FLAG_HBLANK);
         SetHBlankCallback(HBlankCB_Mugshots);
         task->tState++;
         break;
      case TASK_STATE_SHOW_BANNER:
         with_vblank_dma_disabled {
            u16* toStore  = gScanlineEffectRegBuffers[0];
            u8   sinIndex = task->tSinIndex;
            task->tSinIndex += 16;

            // Update top banner
            u8 i = 0;
            for (; i < DISPLAY_HEIGHT / 2; i++, toStore++, sinIndex += 16) {
               s16 x = task->tTopBannerX + Sin(sinIndex, 16);
               if (x < 0)
                  x = 1;
               if (x > DISPLAY_WIDTH)
                  x = DISPLAY_WIDTH;
               *toStore = x;
            }

            // Update bottom banner
            for (; i < DISPLAY_HEIGHT; i++, toStore++, sinIndex += 16) {
               s16 x = task->tBottomBannerX - Sin(sinIndex, 16);
               if (x < 0)
                  x = 0;
               if (x > DISPLAY_WIDTH - 1)
                  x = DISPLAY_WIDTH - 1;
               *toStore = (x << 8) | DISPLAY_WIDTH;
            }

            // Slide banners across screen
            task->tTopBannerX += 8;
            task->tBottomBannerX -= 8;

            if (task->tTopBannerX > DISPLAY_WIDTH)
               task->tTopBannerX = DISPLAY_WIDTH;
            if (task->tBottomBannerX < 0)
               task->tBottomBannerX = 0;

            s32 mergedValue = *(s32*)(&task->tTopBannerX);
            if (mergedValue == DISPLAY_WIDTH)
               task->tState++;

            gBattleTransitionData->BG0HOFS_Lower -= 8;
            gBattleTransitionData->BG0HOFS_Upper += 8;
         }
         break;
      case TASK_STATE_START_OPPONENT_SLIDE:
         with_vblank_dma_disabled {
            u16* toStore = gScanlineEffectRegBuffers[0];
            for (u8 i = 0; i < DISPLAY_HEIGHT; i++, toStore++)
              *toStore = DISPLAY_WIDTH;

            task->tState++;

            // Clear old data
            task->tSinIndex       = 0;
            task->tTopBannerX     = 0;
            task->tBottomBannerX = 0;

            gBattleTransitionData->BG0HOFS_Lower -= 8;
            gBattleTransitionData->BG0HOFS_Upper += 8;

            SetTrainerPicSlideDirection(task->tOpponentSpriteId, 0);
            SetTrainerPicSlideDirection(task->tPlayerSpriteId,   1);

            // Start opponent slide
            IncrementTrainerPicState(task->tOpponentSpriteId);

            PlaySE(SE_MUGSHOT);
         }
         break;
      case TASK_STATE_WAIT_BETWEEN_SLIDES:
         gBattleTransitionData->BG0HOFS_Lower -= 8;
         gBattleTransitionData->BG0HOFS_Upper += 8;
         if (IsTrainerPicSlideDone(task->tOpponentSpriteId)) {
            task->tState++;
            IncrementTrainerPicState(task->tPlayerSpriteId);
         }
         break;
      case TASK_STATE_WAIT_PLAYER_SLIDE:
         gBattleTransitionData->BG0HOFS_Lower -= 8;
         gBattleTransitionData->BG0HOFS_Upper += 8;
         if (IsTrainerPicSlideDone(task->tPlayerSpriteId)) {
            gBattleTransitionData->VBlank_DMA = FALSE;
            SetVBlankCallback(NULL);
            DmaStop(0);
            memset(gScanlineEffectRegBuffers[0], 0, DISPLAY_HEIGHT * 2);
            memset(gScanlineEffectRegBuffers[1], 0, DISPLAY_HEIGHT * 2);
            SetGpuReg(REG_OFFSET_WIN0H, DISPLAY_WIDTH);
            SetGpuReg(REG_OFFSET_BLDY,  0);
            task->tState++;
            task->tTimer      = 0;
            task->tFadeSpread = 0;
            gBattleTransitionData->BLDCNT = BLDCNT_TGT1_ALL | BLDCNT_EFFECT_LIGHTEN;
            SetVBlankCallback(VBlankCB_MugshotsFadeOut);
         }
         break;
      case TASK_STATE_GRADUAL_WHITE_FADE:
         with_vblank_dma_disabled {
            gBattleTransitionData->BG0HOFS_Lower -= 8;
            gBattleTransitionData->BG0HOFS_Upper += 8;

            if (task->tFadeSpread < DISPLAY_HEIGHT / 2)
               task->tFadeSpread += 2;
            if (task->tFadeSpread > DISPLAY_HEIGHT / 2)
               task->tFadeSpread  = DISPLAY_HEIGHT / 2;

            bool32 active = TRUE;
            if (++task->tTimer & 1) {
               active = FALSE;
               for (s16 i = 0; i <= task->tFadeSpread; i++)  {
                     // Fade starts in middle of screen and
                     // spreads outwards in both directions.
                     s16 index1 = DISPLAY_HEIGHT / 2 - i;
                     s16 index2 = DISPLAY_HEIGHT / 2 + i;
                     if (gScanlineEffectRegBuffers[0][index1] <= 15) {
                        active = TRUE;
                        gScanlineEffectRegBuffers[0][index1]++;
                     }
                     if (gScanlineEffectRegBuffers[0][index2] <= 15) {
                        active = TRUE;
                        gScanlineEffectRegBuffers[0][index2]++;
                     }
               }
            }

            if (task->tFadeSpread == DISPLAY_HEIGHT / 2 && !active)
               task->tState++;
         }
         break;
      case TASK_STATE_INIT_FADE_WHITE_TO_BLACK:
         // Set palette to white to replace the scanline white fade
         // before the screen fades to black.
         {
             gBattleTransitionData->VBlank_DMA = FALSE;
             BlendPalettes(PALETTES_ALL, 16, RGB_WHITE);
             gBattleTransitionData->BLDCNT = 0xFF;
             task->tTimer = 0;

             task->tState++;
         }
         break;
      case TASK_STATE_FADE_TO_BLACK:
         with_vblank_dma_disabled {
            task->tTimer++;
            memset(gScanlineEffectRegBuffers[0], task->tTimer, DISPLAY_HEIGHT * 2);
            if (task->tTimer > 15)
               task->tState++;
         }
         break;
      case TASK_STATE_END:
         DmaStop(0);
         BattleTransitionCommon_FadeScreenBlack();
         DestroyTask(FindTaskIdByFunc(task->func));
         break;
   }
}

static void Mugshots_CreateTrainerPics(struct Task* task, const struct BattleTransitionMugshot* params) {
   s16 mugshot_id = task->tMugshotId;
   
   task->tOpponentSpriteId = CreateTrainerSprite(
      params->opponent.trainer_sprite.trainer_pic_id,
      params->opponent.trainer_sprite.coords.x - 32,
      params->opponent.trainer_sprite.coords.y + 42,
      0,
      gDecompressionBuffer
   );
   task->tPlayerSpriteId = CreateTrainerSprite(
      PlayerGenderToFrontTrainerPicId(gSaveBlock2Ptr->playerGender),
      DISPLAY_WIDTH + 32,
      106,
      0,
      gDecompressionBuffer
   );
   
   struct Sprite* sprite_player = &gSprites[task->tPlayerSpriteId];
   struct Sprite* sprite_enemy  = &gSprites[task->tOpponentSpriteId];
   
   sprite_player->callback = sprite_enemy->callback = SpriteCB_MugshotTrainerPic;
   sprite_player->oam.affineMode = sprite_enemy->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
   sprite_player->oam.shape      = sprite_enemy->oam.shape      = SPRITE_SHAPE(64x32);
   sprite_player->oam.size       = sprite_enemy->oam.size       = SPRITE_SIZE(64x32);
   
   sprite_player->oam.matrixNum = AllocOamMatrix();
   sprite_enemy->oam.matrixNum = AllocOamMatrix();
   CalcCenterToCornerVec(
      sprite_enemy,
      SPRITE_SHAPE(64x32),
      SPRITE_SIZE(64x32),
      ST_OAM_AFFINE_DOUBLE
   );
   CalcCenterToCornerVec(
      sprite_player,
      SPRITE_SHAPE(64x32),
      SPRITE_SIZE(64x32),
      ST_OAM_AFFINE_DOUBLE
   );
   
   SetOamMatrixRotationScaling(
      sprite_enemy->oam.matrixNum,
      params->opponent.trainer_sprite.affine.rotation,
      params->opponent.trainer_sprite.affine.scale,
      0
   );
   SetOamMatrixRotationScaling(
      sprite_player->oam.matrixNum,
      -512,
      512,
      0
   );
}

enum SPRITE_STATE {
   SPRITE_STATE_INITIAL_WAIT,
   SPRITE_STATE_INIT,
   SPRITE_STATE_SLIDE_IN,
   SPRITE_STATE_SLIDE_IN_SLOW,
   SPRITE_STATE_PAUSE,
   SPRITE_STATE_SLIDE_OUT,
   SPRITE_STATE_FINAL_WAIT,
};

#define TRAINER_PIC_SLIDE_SPEED (12)
#define TRAINER_PIC_SLIDE_ACCEL (-1)

static void SpriteCB_MugshotTrainerPic(struct Sprite* sprite) {
   switch (sprite->sState) {
      case SPRITE_STATE_INITIAL_WAIT:
      case SPRITE_STATE_PAUSE:
      case SPRITE_STATE_FINAL_WAIT:
         //
         // Wait for outside code to push us to the next state.
         //
         break;
      case SPRITE_STATE_INIT:
         if (sprite->sSlideDir == 0) {
            sprite->sSlideSpeed = TRAINER_PIC_SLIDE_SPEED;
            sprite->sSlideAccel = TRAINER_PIC_SLIDE_ACCEL;
         } else {
            sprite->sSlideSpeed = -TRAINER_PIC_SLIDE_SPEED;
            sprite->sSlideAccel = -TRAINER_PIC_SLIDE_ACCEL;
         }
         sprite->sState++;
         break;
      case SPRITE_STATE_SLIDE_IN:
         sprite->x += sprite->sSlideSpeed;
         
         // Advance state when pic passes ~40% of screen
         if (sprite->sSlideDir && sprite->x < DISPLAY_WIDTH - 107)
            sprite->sState++;
         else if (!sprite->sSlideDir && sprite->x > 103)
            sprite->sState++;
         break;
      case SPRITE_STATE_SLIDE_IN_SLOW:
         // Add acceleration value to speed, then add speed.
         // For both sides acceleration is opposite speed, so slide slows down.
         sprite->sSlideSpeed += sprite->sSlideAccel;
         sprite->x           += sprite->sSlideSpeed;

         // Advance state when slide comes to a stop
         if (sprite->sSlideSpeed == 0) {
            sprite->sState++;
            sprite->sSlideAccel = -sprite->sSlideAccel;
            sprite->sDone = TRUE;
         }
         break;
      case SPRITE_STATE_SLIDE_OUT:
         //
         // This is never reached in vanilla, because we hit another wait state 
         // after SLIDE_IN_SLOW, and outside code never pulls us out of that 
         // wait state (IncrementTrainerPicState is only called once per pic).
         //
         sprite->sSlideSpeed += sprite->sSlideAccel;
         sprite->x           += sprite->sSlideSpeed;
         if (sprite->x < -31 || sprite->x > DISPLAY_WIDTH + 31)
            sprite->sState++;
         break;
   }
}

static void SetTrainerPicSlideDirection(s16 spriteId, s16 dirId) {
   gSprites[spriteId].sSlideDir = dirId;
}

static void IncrementTrainerPicState(s16 spriteId) {
   gSprites[spriteId].sState++;
}

static s16 IsTrainerPicSlideDone(s16 spriteId) {
   return gSprites[spriteId].sDone;
}

static void HBlankCB_Mugshots(void) {
   if (REG_VCOUNT < DISPLAY_HEIGHT / 2)
      REG_BG0HOFS = gBattleTransitionData->BG0HOFS_Lower;
   else
      REG_BG0HOFS = gBattleTransitionData->BG0HOFS_Upper;
}

static void VBlankCB_Mugshots(void) {
   DmaStop(0);
   BattleTransitionCommon_VBlankCB();
   if (gBattleTransitionData->VBlank_DMA != 0)
      DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 2);
   REG_BG0VOFS = gBattleTransitionData->BG0VOFS;
   REG_WININ   = gBattleTransitionData->WININ;
   REG_WINOUT  = gBattleTransitionData->WINOUT;
   REG_WIN0V   = gBattleTransitionData->WIN0V;
   DmaSet(0, gScanlineEffectRegBuffers[1], &REG_WIN0H, B_TRANS_DMA_FLAGS);
}

static void VBlankCB_MugshotsFadeOut(void) {
   DmaStop(0);
   BattleTransitionCommon_VBlankCB();
   if (gBattleTransitionData->VBlank_DMA != 0)
      DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 2);
   REG_BLDCNT = gBattleTransitionData->BLDCNT;
   DmaSet(0, gScanlineEffectRegBuffers[1], &REG_BLDY, B_TRANS_DMA_FLAGS);
}
