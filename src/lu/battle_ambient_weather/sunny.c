#include "lu/battle_ambient_weather/core.h"
#include "lu/battle_ambient_weather/shared.h"
#include "lu/battle_ambient_weather/sunny.h"
#include "global.h" // *sigh*
#include "gba/defines.h"
#include "constants/battle_anim.h" // ANIM_TAG_SUNLIGHT, SOUND_PAN_ATTACKER
#include "constants/rgb.h" // RGB_WHITE
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
#define tState       data[1]
#define tTimer       data[2]
#define tSpriteCount data[3]
enum {
   TASKSTATE_LOAD_GFX,
   TASKSTATE_BACKGROUND_FADE_IN,
   TASKSTATE_LOOPING,
   TASKSTATE_WAITING,
   TASKSTATE_TEARDOWN_REQUESTED,
   TASKSTATE_BACKGROUND_FADE_OUT,
   TASKSTATE_TEARDOWN_WAIT_FOR_SPRITES,
   TASKSTATE_TEARDOWN,
};

// sprite vars
#define sLifespan        data[0]
#define sSetup_XInitial  data[1] // parameters to InitAnimLinearTranslation...
#define sSetup_XTarget   data[2] //
#define sSetup_YInitial  data[3] //
#define sSetup_YTarget   data[4] //
#define sLinear_XDelta   data[1] // AnimTranslateLinear state...
#define sLinear_YDelta   data[2] //
#define sLinear_XCurrent data[3] //
#define sLinear_YCurrent data[4] //
#define sInitialized     data[6]

void StartBattleAmbientWeatherAnim_Sunny(void) {
   gAmbientWeatherTaskId = CreateTask(TaskHandler, 2);
   DebugPrintf("[Battle Ambient Weather][Sunny] Starting, with task ID %u.", gAmbientWeatherTaskId);
}
void StopBattleAmbientWeatherAnim_Sunny(bool8 instant) {
   if (gAmbientWeatherTaskId == TASK_NONE) {
      return;
   }
   struct Task* task = &gTasks[gAmbientWeatherTaskId];
   if (instant) {
      task->tState = TASKSTATE_TEARDOWN;
      return;
   }
   if (task->tState < TASKSTATE_TEARDOWN_REQUESTED) {
      task->tState = TASKSTATE_TEARDOWN_REQUESTED;
   }
}

static void AnimSunlight(struct Sprite* sprite);

static const struct OamData sOamData_AffineNormal_ObjBlend_32x32 = {
   .y = 0,
   .affineMode = ST_OAM_AFFINE_NORMAL,
   .objMode = ST_OAM_OBJ_BLEND,
   .bpp = ST_OAM_4BPP,
   .shape = SPRITE_SHAPE(32x32),
   .x = 0,
   .size = SPRITE_SIZE(32x32),
   .tileNum = 0,
   .priority = 2,
   .paletteNum = 0,
};
static const union AffineAnimCmd sAffineAnim_SunlightRay[] = {
   AFFINEANIMCMD_FRAME(0x50, 0x50, 0, 0),
   AFFINEANIMCMD_FRAME(0x2, 0x2, 10, 1),
   AFFINEANIMCMD_JUMP(1),
};

static const union AffineAnimCmd *const sAffineAnims_SunlightRay[] = {
   sAffineAnim_SunlightRay,
};
static const struct SpriteTemplate sLightRaySpriteTemplate = {
   .tileTag = ANIM_TAG_SUNLIGHT,
   .paletteTag = ANIM_TAG_SUNLIGHT,
   .oam = &sOamData_AffineNormal_ObjBlend_32x32,
   .anims = gDummySpriteAnimTable,
   .images = NULL,
   .affineAnims = sAffineAnims_SunlightRay,
   .callback = AnimSunlight,
};
static const struct CompressedSpriteSheet sLightRaySpriteSheet = {
   gBattleAnimSpriteGfx_Sunlight, 0x0200, ANIM_TAG_SUNLIGHT
};
static const struct CompressedSpritePalette sLightRaySpritePalette = {
   gBattleAnimSpritePal_Sunlight, ANIM_TAG_SUNLIGHT
};

#define SPRITE_OFFSCREEN_X_START 64
#define SPRITE_OFFSCREEN_X_END   32

#define LIGHTRAY_COUNT               4
#define LIGHTRAY_SPRITE_INTERVAL    (6 + 2) // +2 because all `delay` anim commands delay for an extra frame
#define LIGHTRAY_SPRITE_SUBPRIORITY 40
#define LIGHTRAY_GROUP_INTERVAL     110

static u32 GetPalettesToBlend(void) {
   return
      GetBattlePalettesMask(
         TRUE, // battle backgrounds (F_PAL_BG)
         TRUE, // attacker
         TRUE, // target
         TRUE, // attacker partner
         TRUE, // target partner
         FALSE,
         FALSE
      )
      &
      ~(u32)(1 | 2) // avoid messing up the text and textbox-frame palettes
   ;
}

static void TaskHandler(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   
   switch (task->tState) {
      case TASKSTATE_LOAD_GFX:
         // loadspritegfx ANIM_TAG_SUNLIGHT
         LoadCompressedSpriteSheetUsingHeap(&sLightRaySpriteSheet);
         LoadCompressedSpritePaletteUsingHeap(&sLightRaySpritePalette);
         
         // setblend 13 3
         SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_ALL);
         SetGpuReg(REG_OFFSET_BLDALPHA, 13 | (3 << 8));
         
         TaskHelper_QueueBlendColors(
            taskId,
            GetPalettesToBlend(),
            1, // delay
            0, // blend from
            6, // blend to
            RGB_WHITE
         );
         
         ++task->tState;
         DebugPrintf("[Battle Ambient Weather][Sunny] Advancing to TASKSTATE_BACKGROUND_FADE_IN.");
         break;
      case TASKSTATE_BACKGROUND_FADE_IN:
         if (TaskHelper_BlendColors(taskId)) {
            // TODO: panse_adjustnone SE_M_PETAL_DANCE, SOUND_PAN_ATTACKER, SOUND_PAN_TARGET, +1, 0
            PlaySE12WithPanning(SE_M_PETAL_DANCE, SOUND_PAN_ATTACKER);
            
            DebugPrintf("[Battle Ambient Weather][Sunny] Advancing to TASKSTATE_LOOPING.");
            ++task->tState;
         }
         break;
      case TASKSTATE_LOOPING:
         if ((task->tTimer % LIGHTRAY_SPRITE_INTERVAL) == 0) {
            // createsprite gSunlightRaySpriteTemplate, ANIM_ATTACKER, 40
            u8 spriteId = CreateSprite(
               &sLightRaySpriteTemplate,
               GetBattlerSpriteCoord(0, BATTLER_COORD_X_2),
               GetBattlerSpriteCoord(0, BATTLER_COORD_Y_PIC_OFFSET),
               GetBattlerSpriteSubpriority(0) - 40
            );
            gSprites[spriteId].sInitialized = FALSE;
            gSprites[spriteId].callback(&gSprites[spriteId]);
            AnimateSprite(&gSprites[spriteId]);
            
            if (++task->tSpriteCount >= LIGHTRAY_COUNT) {
               ++task->tState;
               break;
            }
         }
         ++task->tTimer;
         break;
      case TASKSTATE_WAITING:
         ++task->tTimer;
         if (task->tTimer >= LIGHTRAY_GROUP_INTERVAL) {
            task->tTimer = 0;
            --task->tState;
         }
         break;
      case TASKSTATE_TEARDOWN_REQUESTED:
         TaskHelper_QueueBlendColors(
            taskId,
            GetPalettesToBlend(),
            1, // delay
            6, // blend from
            0, // blend to
            RGB_WHITE
         );
         ++task->tState;
         DebugPrintf("[Battle Ambient Weather][Sunny] Advancing to TASKSTATE_BACKGROUND_FADE_OUT.");
         break;
      case TASKSTATE_BACKGROUND_FADE_OUT: // Vanilla AnimTask_LoadSandstormBackground, state 2
         if (TaskHelper_BlendColors(taskId)) {
            // blendoff
            SetGpuReg(REG_OFFSET_BLDCNT, 0);
            SetGpuReg(REG_OFFSET_BLDALPHA, 0);
            
            DebugPrintf("[Battle Ambient Weather][Sunny] Advancing to TASKSTATE_TEARDOWN_WAIT_FOR_SPRITES.");
            ++task->tState;
         }
         break;
      case TASKSTATE_TEARDOWN_WAIT_FOR_SPRITES:
         if (task->tSpriteCount <= 0) {
            DebugPrintf("[Battle Ambient Weather][Sunny] Advancing to TASKSTATE_TEARDOWN.");
            ++task->tState;
         }
         break;
      case TASKSTATE_TEARDOWN:
         // Do these again, so that they don't get forgotten for immediate teardowns.
         SetGpuReg(REG_OFFSET_BLDCNT, 0);
         SetGpuReg(REG_OFFSET_BLDALPHA, 0);
         
         DebugPrintf("[Battle Ambient Weather][Sunny] Destroying task.");
         DestroyTask(taskId);
         gAmbientWeatherTaskId = TASK_NONE;
         return;
   }
}

static void AnimSunlight(struct Sprite *sprite) {
   if (gAmbientWeatherTaskId == TASK_NONE) {
      //DebugPrintf("[Battle Ambient Weather][Sunny] Destroying a sprite.");
      FreeSpriteOamMatrix(sprite);
      DestroySprite(sprite);
      return;
   }
   if (sprite->sInitialized == 0) {
      sprite->x = 0;
      sprite->y = 0;
      sprite->sLifespan      =  60;
      sprite->sSetup_XTarget = 140;
      sprite->sSetup_YTarget =  80;
      InitAnimLinearTranslation(sprite);
      sprite->sInitialized = TRUE;
      return;
   }
   if (AnimTranslateLinear(sprite)) {
      //DebugPrintf("[Battle Ambient Weather][Sunny] Destroying a sprite.");
      FreeSpriteOamMatrix(sprite);
      DestroySprite(sprite);
      --gTasks[gAmbientWeatherTaskId].tSpriteCount;
   }
}