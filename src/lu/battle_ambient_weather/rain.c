#include "lu/battle_ambient_weather/core.h"
#include "lu/battle_ambient_weather/shared.h"
#include "lu/battle_ambient_weather/rain.h"
#include "global.h" // *sigh*
#include "gba/defines.h"
#include "constants/battle_anim.h" // ANIM_TAG_RAIN_DROPS, SOUND_PAN_ATTACKER
#include "constants/rgb.h" // RGB_BLACK
#include "constants/songs.h"
#include "decompress.h"
#include "random.h"
#include "sound.h" // PlaySE12WithPanning
#include "sprite.h"
#include "task.h"

#include "graphics.h" // dependency for next header? (TODO: re-check that)
#include "battle_anim.h" // GetBattlePalettesMask

#include "gba/isagbprint.h"

static void TaskHandler(u8 taskId);

// task vars
#define tState       data[1]
#define tTimer       data[2]
#define tSpriteCount data[3]
enum {
   TASKSTATE_LOAD_GFX,
   TASKSTATE_DARKEN_BACKGROUND,
   TASKSTATE_INITIAL_PERIOD,
   TASKSTATE_LOOPING,
   TASKSTATE_TEARDOWN_REQUESTED,
   TASKSTATE_TEARDOWN_LIGHTEN_BACKGROUND,
   TASKSTATE_TEARDOWN_WAIT_FOR_SPRITES,
   TASKSTATE_TEARDOWN,
};

void StartBattleAmbientWeatherAnim_Rain(void) {
   gAmbientWeatherTaskId = CreateTask(TaskHandler, 2);
   DebugPrintf("[Battle Ambient Weather][Rain] Starting, with task ID %u.", gAmbientWeatherTaskId);
}
void StopBattleAmbientWeatherAnim_Rain(bool8 instant) {
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

static void AnimRainDrop(struct Sprite*);
//
static const union AnimCmd sAnim_RainDrop[] = {
   ANIMCMD_FRAME( 0, 2), // imageValue, duration
   ANIMCMD_FRAME( 8, 2),
   ANIMCMD_FRAME(16, 2),
   ANIMCMD_FRAME(24, 6),
   ANIMCMD_FRAME(32, 2),
   ANIMCMD_FRAME(40, 2), // drop stops moving (i.e. splashes) at this point
   ANIMCMD_FRAME(48, 2),
   ANIMCMD_END, // total: 18 frames
};
static const union AnimCmd *const sAnims_RainDrop[] = {
   sAnim_RainDrop,
};
static const struct OamData sOamData_AffineOff_ObjNormal_16x32 = {
   .y = 0,
   .affineMode = ST_OAM_AFFINE_OFF,
   .objMode = ST_OAM_OBJ_NORMAL,
   .bpp = ST_OAM_4BPP,
   .shape = SPRITE_SHAPE(16x32),
   .x = 0,
   .size = SPRITE_SIZE(16x32),
   .tileNum = 0,
   .priority = 2,
   .paletteNum = 0,
};
static const struct SpriteTemplate sRainDropSpriteTemplate = {
   .tileTag     = ANIM_TAG_RAIN_DROPS,
   .paletteTag  = ANIM_TAG_RAIN_DROPS,
   .oam         = &sOamData_AffineOff_ObjNormal_16x32,
   .anims       = sAnims_RainDrop,
   .images      = NULL,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback    = AnimRainDrop,
};
static const struct CompressedSpriteSheet sRainDropSpriteSheet = {
   gBattleAnimSpriteGfx_RainDrops, 0x0700, ANIM_TAG_RAIN_DROPS
};
static const struct CompressedSpritePalette sRainDropSpritePalette = {
   gBattleAnimSpritePal_RainDrops, ANIM_TAG_RAIN_DROPS
};

#define SPAWN_DROP_INTERVAL 3
#define SPAWN_DROP_DURATION 60
#define MAX_DROP_COUNT (SPAWN_DROP_DURATION / SPAWN_DROP_INTERVAL)

// sprite vars
#define sTimer data[0]

static void SpawnDrop(u8 taskId) {
   u8 x        = Random2() % DISPLAY_WIDTH;
   u8 y        = Random2() % (DISPLAY_HEIGHT / 2);
   u8 spriteId = CreateSprite(&sRainDropSpriteTemplate, x, y, 4);
   gTasks[taskId].tSpriteCount++;
}
static void ResetDrop(struct Sprite* sprite) {
   sprite->x      = Random2() % DISPLAY_WIDTH;
   sprite->y      = Random2() % (DISPLAY_HEIGHT / 2);
   sprite->x2     = 0;
   sprite->y2     = 0;
   sprite->sTimer = 0;
   SeekSpriteAnim(sprite, 0);
}

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
         LoadCompressedSpriteSheetUsingHeap(&sRainDropSpriteSheet);
         LoadCompressedSpritePaletteUsingHeap(&sRainDropSpritePalette);
         TaskHelper_QueueBlendColors(
            taskId,
            GetPalettesToBlend(),
            2, // delay
            0, // blend from
            4, // blend to
            RGB_BLACK
         );
         DebugPrintf("[Battle Ambient Weather][Rain] Advancing to TASKSTATE_DARKEN_BACKGROUND.");
         PlaySE12WithPanning(SE_M_RAIN_DANCE, SOUND_PAN_ATTACKER);
         ++task->tState;
         break;
      case TASKSTATE_DARKEN_BACKGROUND:
         if (TaskHelper_BlendColors(taskId)) {
            DebugPrintf("[Battle Ambient Weather][Rain] Advancing to TASKSTATE_INITIAL_PERIOD.");
            ++task->tState;
         }
         break;
      case TASKSTATE_INITIAL_PERIOD:
         //
         // Vanilla creates two concurrent raindrop tasks. Each task spawns 
         // one raindrop in every three-frame window, over a sixty-frame 
         // period. This gives us a total of 40 raindrop sprites spawned.
         //
         // Raindrops have an 18-frame animation, and destroy themselves 
         // after the animation ends.
         //
         // As such, the total number of extant sprites at frame F is:
         //
         //    S = 2 * (floor((F + 2) / 3) - 2*floor((F - 1) / 18))
         //
         // The game creates sprites faster than it destroys them, and by 
         // the end of the vanilla effect, roughly 28 sprites are present. 
         // If we loop this effect indefinitely, then we'll eventually 
         // blow the GBA's sprite budget.
         //
         ++task->tTimer;
         if (task->tTimer % SPAWN_DROP_INTERVAL == 1) {
            SpawnDrop(taskId);
            SpawnDrop(taskId);
         }
         if (task->tTimer >= SPAWN_DROP_DURATION) {
            DebugPrintf("[Battle Ambient Weather][Rain] Advancing to TASKSTATE_LOOPING.");
            ++task->tState;
         }
         break;
      case TASKSTATE_LOOPING:
         ++task->tTimer;
         if (task->tTimer % SPAWN_DROP_INTERVAL == 1) {
            if (task->tSpriteCount < MAX_DROP_COUNT) {
               SpawnDrop(taskId);
               if (task->tSpriteCount < MAX_DROP_COUNT) {
                  SpawnDrop(taskId);
               }
            }
         }
         break;
      case TASKSTATE_TEARDOWN_REQUESTED:
         TaskHelper_QueueBlendColors(
            taskId,
            GetPalettesToBlend(),
            1, // delay
            4, // blend from
            0, // blend to
            RGB_BLACK
         );
         ++task->tState;
         DebugPrintf("[Battle Ambient Weather][Rain] Advancing to TASKSTATE_TEARDOWN_LIGHTEN_BACKGROUND.");
         break;
      case TASKSTATE_TEARDOWN_LIGHTEN_BACKGROUND:
         if (TaskHelper_BlendColors(taskId)) {
            DebugPrintf("[Battle Ambient Weather][Rain] Advancing to TASKSTATE_TEARDOWN_WAIT_FOR_SPRITES.");
            ++task->tState;
         }
         break;
      case TASKSTATE_TEARDOWN_WAIT_FOR_SPRITES:
         if (task->tSpriteCount <= 0) {
            DebugPrintf("[Battle Ambient Weather][Rain] Advancing to TASKSTATE_TEARDOWN.");
            ++task->tState;
         }
         break;
      case TASKSTATE_TEARDOWN:
         DebugPrintf("[Battle Ambient Weather][Rain] Destroying task.");
         DestroyTask(taskId);
         gAmbientWeatherTaskId = TASK_NONE;
         break;
   }
}

static void AnimRainDrop(struct Sprite* sprite) {
   if (gAmbientWeatherTaskId == TASK_NONE) {
      DestroySprite(sprite);
      return;
   }
   sprite->sTimer++;
   if (sprite->sTimer > 1 && sprite->sTimer < 14) {
      sprite->x2++;
      sprite->y2 += 4;
   }
   if (sprite->animEnded) {
      struct Task* task = &gTasks[gAmbientWeatherTaskId];
      if (task->tState == TASKSTATE_LOOPING) {
         ResetDrop(sprite);
      } else {
         --task->tSpriteCount;
         DestroySprite(sprite);
      }
   }
}

#undef sTimer