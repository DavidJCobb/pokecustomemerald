#include "lu/battle_ambient_weather/core.h"
#include "lu/battle_ambient_weather/shared.h"
#include "lu/battle_ambient_weather/rain.h"
#include "global.h" // *sigh*
#include "gba/defines.h"
#include "constants/battle_anim.h" // ANIM_TAG_..., SOUND_PAN_ATTACKER
#include "constants/songs.h"
#include "decompress.h"
#include "gpu_regs.h" // SetGpuReg
#include "palette.h" // LoadCompressedPalette
#include "random.h"
#include "sound.h" // PlaySE12WithPanning
#include "sprite.h"
#include "task.h"

#include "graphics.h" // gBattleAnimBgImage_Sandstorm and friends
#include "battle_anim.h" // GetBattlePalettesMask

#include "gba/isagbprint.h"

static void TaskHandler(u8 taskId);

// task vars
#define tState             data[1]
#define tDirection         data[2]
#define tTimer             data[3]
#define tBlend             data[4] // data[11] in vanilla AnimTask_LoadSandstormBackground
#define tBlendTimer        data[5] // data[10] in vanilla AnimTask_LoadSandstormBackground
#define tSpriteCount       data[6]
#define tTeardownImmediate data[7]
enum {
   TASKSTATE_LOAD_GFX,
   TASKSTATE_BACKGROUND_BLEND_IN, // Vanilla AnimTask_LoadSandstormBackground, state 0
   TASKSTATE_FULL_SAND_OPACITY, // Vanilla AnimTask_LoadSandstormBackground, state 1
   TASKSTATE_LOOPING,
   TASKSTATE_TEARDOWN_REQUESTED,
   TASKSTATE_BACKGROUND_BLEND_OUT, // Vanilla AnimTask_LoadSandstormBackground, state 2
   TASKSTATE_BACKGROUND_ANIM_TEARDOWN, // Vanilla AnimTask_LoadSandstormBackground, state 3
   TASKSTATE_BACKGROUND_FULL_TEARDOWN, // Vanilla AnimTask_LoadSandstormBackground, state 4
   TASKSTATE_TEARDOWN_WAIT_FOR_SPRITES,
   TASKSTATE_TEARDOWN,
};

// sprite vars
#define sState       data[0]
#define sVelocityX   data[1] // 256ths of a pixel
#define sVelocityY   data[2] // 256ths of a pixel
#define sFractionalX data[3] // 256ths of a pixel
#define sFractionalY data[4] // 256ths of a pixel
#define sMirroredX   data[5]

void StartBattleAmbientWeatherAnim_Sandstorm(void) {
   gAmbientWeatherTaskId = CreateTask(TaskHandler, 2);
   DebugPrintf("[Battle Ambient Weather][Sandstorm] Starting, with task ID %u.", gAmbientWeatherTaskId);
}
void StopBattleAmbientWeatherAnim_Sandstorm(bool8 instant) {
   if (gAmbientWeatherTaskId == TASK_NONE) {
      return;
   }
   struct Task* task = &gTasks[gAmbientWeatherTaskId];
   task->tTeardownImmediate |= instant;
   if (task->tState < TASKSTATE_TEARDOWN_REQUESTED) {
      task->tState = TASKSTATE_TEARDOWN_REQUESTED;
   }
}

static void AnimFlyingSandCrescent(struct Sprite* sprite);
//
static const struct OamData sOamData_AffineOff_ObjNormal_32x16 = {
   .y = 0,
   .affineMode = ST_OAM_AFFINE_OFF,
   .objMode = ST_OAM_OBJ_NORMAL,
   .bpp = ST_OAM_4BPP,
   .shape = SPRITE_SHAPE(32x16),
   .x = 0,
   .size = SPRITE_SIZE(32x16),
   .tileNum = 0,
   .priority = 2,
   .paletteNum = 0,
};
static const struct SpriteTemplate sFlyingSandCrescentSpriteTemplate = {
   .tileTag     = ANIM_TAG_FLYING_DIRT,
   .paletteTag  = ANIM_TAG_FLYING_DIRT,
   .oam         = &sOamData_AffineOff_ObjNormal_32x16,
   .anims       = gDummySpriteAnimTable,
   .images      = NULL,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback    = AnimFlyingSandCrescent,
};
static const struct CompressedSpriteSheet sFlyingSandSpriteSheet = {
   gBattleAnimSpriteGfx_FlyingDirt, 0x0200, ANIM_TAG_FLYING_DIRT
};
static const struct CompressedSpritePalette sFlyingSandSpritePalette = {
   gBattleAnimSpritePal_FlyingDirt, ANIM_TAG_FLYING_DIRT
};
//
static const struct Subsprite sFlyingSandSubsprites[] = {
   { .x = -16, .y = 0, .shape = SPRITE_SHAPE(32x16), .size = SPRITE_SIZE(32x16), .tileOffset = 0, .priority = 1 },
   { .x =  16, .y = 0, .shape = SPRITE_SHAPE(32x16), .size = SPRITE_SIZE(32x16), .tileOffset = 8, .priority = 1 },
};
static const struct SubspriteTable sFlyingSandSubspriteTable[] = {
   { ARRAY_COUNT(sFlyingSandSubsprites), sFlyingSandSubsprites },
};
#define SPRITE_OFFSCREEN_X_START 64
#define SPRITE_OFFSCREEN_X_END   32

static const struct {
   u8    frame;
   u8    subpriority;
   u8    initial_y;
   u16   speed;   // measured in 256ths of a pixel
   u16   gravity; // measured in 256ths of a pixel
   bool8 flip_horizontally;
} sCloudParams[] = {
   { 16, 40, 10, 2304, 96, 0 },
   { 26, 40, 90, 2048, 96, 0 },
   { 36, 40, 50, 2560, 96, 0 },
	{ 46, 40, 20, 2304, 96, 0 },
	{ 56, 40, 70, 1984, 96, 0 },
	{ 66, 40,  0, 2816, 96, 0 },
	{ 76, 40, 60, 2560, 96, 0 },
};

static void TaskHandler(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   if (task->tState > TASKSTATE_LOAD_GFX) {
      ++task->tTimer;
   }
   if (task->tState >= TASKSTATE_BACKGROUND_BLEND_IN) {
      if (task->tDirection == 0)
         gBattle_BG1_X += -6;
      else
         gBattle_BG1_X += 6;
      gBattle_BG1_Y += -1;
   }
   
   struct BattleAnimBgData animBg;
   switch (task->tState) {
      case TASKSTATE_LOAD_GFX:
         // loadspritegfx ANIM_TAG_FLYING_DIRT
         LoadCompressedSpriteSheetUsingHeap(&sFlyingSandSpriteSheet);
         LoadCompressedSpritePaletteUsingHeap(&sFlyingSandSpritePalette);
         #if ENABLE_LOOPING_AMBIENT_SFX
            SpawnSELoopTask(
               taskId,
               SE_M_SANDSTORM,
               SOUND_PAN_ATTACKER,
               330,
               0xFFFF
            );
         #else
            // playsewithpan SE_M_SANDSTORM, 0
            PlaySE12WithPanning(SE_M_SANDSTORM, SOUND_PAN_ATTACKER);
         #endif
         {  // Vanilla: AnimTask_LoadSandstormBackground
            SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG1 | BLDCNT_TGT2_ALL | BLDCNT_EFFECT_BLEND);
            SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(0, 16));
            SetAnimBgAttribute(1, BG_ANIM_PRIORITY, 1);
            SetAnimBgAttribute(1, BG_ANIM_SCREEN_SIZE, 0);
            if (!IsContest())
               SetAnimBgAttribute(1, BG_ANIM_CHAR_BASE_BLOCK, 1);
            gBattle_BG1_X = 0;
            gBattle_BG1_Y = 0;
            SetGpuReg(REG_OFFSET_BG1HOFS, gBattle_BG1_X);
            SetGpuReg(REG_OFFSET_BG1VOFS, gBattle_BG1_Y);

            GetBattleAnimBg1Data(&animBg);
            AnimLoadCompressedBgGfx(animBg.bgId, gBattleAnimBgImage_Sandstorm, animBg.tilesOffset);
            AnimLoadCompressedBgTilemapHandleContest(&animBg, gBattleAnimBgTilemap_Sandstorm, FALSE);
            LoadCompressedPalette(gBattleAnimSpritePal_FlyingDirt, BG_PLTT_ID(animBg.paletteId), PLTT_SIZE_4BPP);
            
            task->tDirection = 0; // TODO: Use 0 or 1?
         }
         ++task->tState;
         DebugPrintf("[Battle Ambient Weather][Sandstorm] Advancing to TASKSTATE_BACKGROUND_BLEND_IN.");
         break;
      case TASKSTATE_BACKGROUND_BLEND_IN: // Vanilla AnimTask_LoadSandstormBackground, state 0
         if (++gTasks[taskId].tBlendTimer == 4) {
            task->tBlendTimer = 0;
            task->tBlend++;
            SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(task->tBlend, 16 - task->tBlend));
            if (task->tBlend == 7) {
               ++task->tState;
               task->tBlend      = 0;
               task->tBlendTimer = 0;
               DebugPrintf("[Battle Ambient Weather][Sandstorm] Advancing to TASKSTATE_FULL_SAND_OPACITY.");
            }
         }
         break;
      case TASKSTATE_FULL_SAND_OPACITY: // Vanilla AnimTask_LoadSandstormBackground, state 1
         task->tBlend      = 7;
         task->tBlendTimer = 0;
         ++task->tState;
         DebugPrintf("[Battle Ambient Weather][Sandstorm] Advancing to TASKSTATE_LOOPING.");
         break;
      case TASKSTATE_LOOPING:
         ;
         break;
      case TASKSTATE_TEARDOWN_REQUESTED:
         ++task->tState;
         DebugPrintf("[Battle Ambient Weather][Sandstorm] Advancing to TASKSTATE_BACKGROUND_BLEND_OUT.");
         break;
      case TASKSTATE_BACKGROUND_BLEND_OUT: // Vanilla AnimTask_LoadSandstormBackground, state 2
         if (task->tTeardownImmediate) {
            DebugPrintf("[Battle Ambient Weather][Sandstorm] Advancing to TASKSTATE_BACKGROUND_ANIM_TEARDOWN.");
            ++task->tState;
            break;
         }
         if (++task->tBlendTimer == 4) {
            task->tBlendTimer = 0;
            task->tBlend--;
            SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(task->tBlend, 16 - task->tBlend));
            if (task->tBlend == 0) {
               ++task->tState;
               task->tBlend = 0;
               DebugPrintf("[Battle Ambient Weather][Sandstorm] Advancing to TASKSTATE_BACKGROUND_ANIM_TEARDOWN.");
            }
         }
         break;
      case TASKSTATE_BACKGROUND_ANIM_TEARDOWN: // Vanilla AnimTask_LoadSandstormBackground, state 3
         GetBattleAnimBg1Data(&animBg);
         ClearBattleAnimBg(animBg.bgId);
         #if ENABLE_LOOPING_AMBIENT_SFX
            DestroySELoopTask(taskId);
         #endif
         ++task->tState;
         DebugPrintf("[Battle Ambient Weather][Sandstorm] Advancing to TASKSTATE_BACKGROUND_FULL_TEARDOWN.");
         break;
      case TASKSTATE_BACKGROUND_FULL_TEARDOWN: // Vanilla AnimTask_LoadSandstormBackground, state 4
         if (!IsContest())
            SetAnimBgAttribute(1, BG_ANIM_CHAR_BASE_BLOCK, 0);

         gBattle_BG1_X = 0;
         gBattle_BG1_Y = 0;
         SetGpuReg(REG_OFFSET_BLDCNT, 0);
         SetGpuReg(REG_OFFSET_BLDALPHA, 0);
         SetAnimBgAttribute(1, BG_ANIM_PRIORITY, 1);
         
         ++task->tState;
         DebugPrintf("[Battle Ambient Weather][Sandstorm] Advancing to TASKSTATE_TEARDOWN_WAIT_FOR_SPRITES.");
         // [[fallthrough]];
      case TASKSTATE_TEARDOWN_WAIT_FOR_SPRITES:
         if (task->tTeardownImmediate || task->tSpriteCount <= 0) {
            DebugPrintf("[Battle Ambient Weather][Sandstorm] Advancing to TASKSTATE_TEARDOWN.");
            ++task->tState;
         }
         break;
      case TASKSTATE_TEARDOWN:
         DebugPrintf("[Battle Ambient Weather][Sandstorm] Destroying task.");
         DestroyTask(taskId);
         gAmbientWeatherTaskId = TASK_NONE;
         return;
   }
   
   if (task->tState < TASKSTATE_TEARDOWN_REQUESTED) {
      for(u8 i = 0; i < ARRAY_COUNT(sCloudParams); ++i) {
         if (task->tTimer == sCloudParams[i].frame) {
            u8 spriteId = CreateSprite(
               &sFlyingSandCrescentSpriteTemplate,
               sCloudParams[i].flip_horizontally ? DISPLAY_WIDTH + SPRITE_OFFSCREEN_X_START : -SPRITE_OFFSCREEN_X_START,
               sCloudParams[i].initial_y,
               sCloudParams[i].subpriority
            );
            gSprites[spriteId].sVelocityX = sCloudParams[i].speed;
            gSprites[spriteId].sVelocityY = sCloudParams[i].gravity;
            gSprites[spriteId].sMirroredX = sCloudParams[i].flip_horizontally;
            if (sCloudParams[i].flip_horizontally) {
               gSprites[spriteId].oam.matrixNum = ST_OAM_HFLIP;
            }
            ++task->tSpriteCount;
            break;
         }
      }
   }
}

static void AnimFlyingSandCrescent(struct Sprite *sprite) {
   if (gAmbientWeatherTaskId == TASK_NONE) {
      DebugPrintf("[Battle Ambient Weather][Sandstorm] Destroying a sprite.");
      FreeSpriteOamMatrix(sprite);
      DestroySprite(sprite);
      return;
   }
   if (sprite->sState == 0) {
      SetSubspriteTables(sprite, sFlyingSandSubspriteTable);
      sprite->sState++;
   } else {
      sprite->sFractionalX += sprite->sVelocityX;
      sprite->sFractionalY += sprite->sVelocityY;
      sprite->x2 += (sprite->sFractionalX >> 8);
      sprite->y2 += (sprite->sFractionalY >> 8);
      sprite->sFractionalX &= 0xFF;
      sprite->sFractionalY &= 0xFF;

      bool8 offscreen = FALSE;
      if (sprite->sMirroredX == 0) {
         if (sprite->x + sprite->x2 > DISPLAY_WIDTH + SPRITE_OFFSCREEN_X_END) {
            offscreen = TRUE;
         }
      } else if (sprite->x + sprite->x2 < -SPRITE_OFFSCREEN_X_END) {
         offscreen = TRUE;
      }
      if (offscreen) {
         struct Task* task = &gTasks[gAmbientWeatherTaskId];
         if (task->tState >= TASKSTATE_TEARDOWN_REQUESTED) {
            DebugPrintf("[Battle Ambient Weather][Sandstorm] Destroying a sprite.");
            FreeSpriteOamMatrix(sprite);
            DestroySprite(sprite);
            task->tSpriteCount--;
         } else {
            //
            // Reuse the sprite.
            //
            sprite->x2 = 0;
            sprite->y2 = 0;
            sprite->sFractionalX = 0;
            sprite->sFractionalY = 0;
         }
      }
   }
}