#include "lu/battle_ambient_weather/core.h"
#include "lu/battle_ambient_weather/shared.h"
#include "lu/battle_ambient_weather/hail.h"
#include "global.h" // *sigh*
#include "gba/defines.h"
#include "constants/battle_anim.h" // ANIM_TAG_HAIL, ANIM_TAG_ICE_CRYSTALS, SOUND_PAN_ATTACKER
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

void StartBattleAmbientWeatherAnim_Hail(void) {
   gAmbientWeatherTaskId = CreateTask(TaskHandler, 2);
   DebugPrintf("[Battle Ambient Weather][Hail] Starting, with task ID %u.", gAmbientWeatherTaskId);
}
void StopBattleAmbientWeatherAnim_Hail(bool8 instant) {
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

// Hail sprite START
static void AnimHailBegin(struct Sprite*);

static const struct OamData sOamData_AffineNormal_ObjNormal_16x16 = {
   .y = 0,
   .affineMode = ST_OAM_AFFINE_NORMAL,
   .objMode = ST_OAM_OBJ_NORMAL,
   .bpp = ST_OAM_4BPP,
   .shape = SPRITE_SHAPE(16x16),
   .x = 0,
   .size = SPRITE_SIZE(16x16),
   .tileNum = 0,
   .priority = 2,
   .paletteNum = 0,
};

static const union AffineAnimCmd sAffineAnim_HailParticle_0[] = {
   AFFINEANIMCMD_FRAME(0x100, 0x100, 0, 0),
   AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAffineAnim_HailParticle_1[] = {
   AFFINEANIMCMD_FRAME(0xF0, 0xF0, 0, 0),
   AFFINEANIMCMD_END,
};
static const union AffineAnimCmd sAffineAnim_HailParticle_2[] = {
   AFFINEANIMCMD_FRAME(0xE0, 0xE0, 0, 0),
   AFFINEANIMCMD_END,
};
static const union AffineAnimCmd* const sAffineAnims_HailParticle[] = {
   sAffineAnim_HailParticle_0,
   sAffineAnim_HailParticle_1,
   sAffineAnim_HailParticle_2,
};

static const struct SpriteTemplate sHailParticleSpriteTemplate = {
   .tileTag     = ANIM_TAG_HAIL,
   .paletteTag  = ANIM_TAG_HAIL,
   .oam         = &sOamData_AffineNormal_ObjNormal_16x16,
   .anims       = gDummySpriteAnimTable,
   .images      = NULL,
   .affineAnims = sAffineAnims_HailParticle,
   .callback    = AnimHailBegin,
};

static const struct CompressedSpriteSheet sHailParticleSpriteSheet = {
   gBattleAnimSpriteGfx_Hail, 0x0080, ANIM_TAG_HAIL
};
static const struct CompressedSpritePalette sHailParticleSpritePalette = {
   gBattleAnimSpritePal_Hail, ANIM_TAG_HAIL
};
// Hail sprite END

// Ice crystals START
static const struct OamData sOamData_AffineNormal_ObjBlend_8x16 = {
   .y = 0,
   .affineMode = ST_OAM_AFFINE_NORMAL,
   .objMode = ST_OAM_OBJ_BLEND,
   .bpp = ST_OAM_4BPP,
   .shape = SPRITE_SHAPE(8x16),
   .x = 0,
   .size = SPRITE_SIZE(8x16),
   .tileNum = 0,
   .priority = 2,
   .paletteNum = 0,
};

static const union AffineAnimCmd sAffineAnim_IceCrystalHit[] = {
   AFFINEANIMCMD_FRAME(0xCE, 0xCE, 0, 0),
   AFFINEANIMCMD_FRAME(0x5, 0x5, 0, 10),
   AFFINEANIMCMD_FRAME(0x0, 0x0, 0, 6),
   AFFINEANIMCMD_END,
};

static const union AffineAnimCmd *const sAffineAnims_IceCrystalHit[] = {
   sAffineAnim_IceCrystalHit,
};

static const struct SpriteTemplate sIceCrystalHitLargeSpriteTemplate = {
   .tileTag     = ANIM_TAG_ICE_CRYSTALS,
   .paletteTag  = ANIM_TAG_ICE_CRYSTALS,
   .oam         = &sOamData_AffineNormal_ObjBlend_8x16,
   .anims       = sAnims_IceCrystalLarge,
   .images      = NULL,
   .affineAnims = sAffineAnims_IceCrystalHit,
   .callback    = AnimIceEffectParticle,
};

static const struct CompressedSpriteSheet sIceCrystalsSpriteSheet = {
   gBattleAnimSpriteGfx_IceCrystals, 0x01c0, ANIM_TAG_ICE_CRYSTALS
};
static const struct CompressedSpritePalette sIceCrystalsSpritePalette = {
   gBattleAnimSpritePal_IceCrystals, ANIM_TAG_ICE_CRYSTALS
};
// Ice crystals END

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
         // loadspritegfx ANIM_TAG_HAIL
         LoadCompressedSpriteSheetUsingHeap(&sHailParticleSpriteSheet);
         LoadCompressedSpritePaletteUsingHeap(&sHailParticleSpritePalette);
         // loadspritegfx ANIM_TAG_ICE_CRYSTALS
         LoadCompressedSpriteSheetUsingHeap(&sIceCrystalsSpriteSheet);
         LoadCompressedSpritePaletteUsingHeap(&sIceCrystalsSpritePalette);
         
         TaskHelper_QueueBlendColors(
            taskId,
            GetPalettesToBlend(),
            3, // delay
            0, // blend from
            6, // blend to
            RGB_BLACK
         );
         DebugPrintf("[Battle Ambient Weather][Hail] Advancing to TASKSTATE_DARKEN_BACKGROUND.");
         PlaySE12WithPanning(SE_M_RAIN_DANCE, SOUND_PAN_ATTACKER);
         ++task->tState;
         break;
      case TASKSTATE_DARKEN_BACKGROUND:
         if (TaskHelper_BlendColors(taskId)) {
            DebugPrintf("[Battle Ambient Weather][Hail] Advancing to TASKSTATE_INITIAL_PERIOD.");
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
            DebugPrintf("[Battle Ambient Weather][Hail] Advancing to TASKSTATE_LOOPING.");
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
            6, // blend from
            0, // blend to
            RGB_BLACK
         );
         ++task->tState;
         DebugPrintf("[Battle Ambient Weather][Hail] Advancing to TASKSTATE_TEARDOWN_LIGHTEN_BACKGROUND.");
         break;
      case TASKSTATE_TEARDOWN_LIGHTEN_BACKGROUND:
         if (TaskHelper_BlendColors(taskId)) {
            DebugPrintf("[Battle Ambient Weather][Hail] Advancing to TASKSTATE_TEARDOWN_WAIT_FOR_SPRITES.");
            ++task->tState;
         }
         break;
      case TASKSTATE_TEARDOWN_WAIT_FOR_SPRITES:
         if (task->tSpriteCount <= 0) {
            DebugPrintf("[Battle Ambient Weather][Hail] Advancing to TASKSTATE_TEARDOWN.");
            ++task->tState;
         }
         break;
      case TASKSTATE_TEARDOWN:
         DebugPrintf("[Battle Ambient Weather][Hail] Destroying task.");
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