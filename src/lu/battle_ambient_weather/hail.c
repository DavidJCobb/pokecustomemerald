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
#include "battle_anim.h" // GetBattlePalettesMask, GetBattlerSpriteCoordAttr

#include "gba/isagbprint.h"

#define FALLING_HAIL_INITIAL_DELAY 2
#define ENABLE_HAIL_IMPACT_VFX 1
#define HAIL_IMPACT_VFX_LIFESPAN 20

enum {
   HAILSTRUCTTYPE_NEGATIVE_POS_MOD = 0,
   HAILSTRUCTTYPE_POSITIVE_POS_MOD = 1,
   HAILSTRUCTTYPE_FIXED_POSITION   = 2,
};
struct HailStruct {
   s32 x:10;
   s32 y:10;
   s32 bPosition:8;
   s32 type:4;
};
static const struct HailStruct sHailCoordData[] = {
   {
      .x = 100,
      .y = 120,
      .bPosition = B_POSITION_PLAYER_LEFT,
      .type = HAILSTRUCTTYPE_FIXED_POSITION
   },
   {
      .x =  85,
      .y = 120,
      .bPosition = B_POSITION_PLAYER_LEFT,
      .type = HAILSTRUCTTYPE_NEGATIVE_POS_MOD
   },
   {
      .x = 242,
      .y = 120,
      .bPosition = B_POSITION_OPPONENT_LEFT,
      .type = HAILSTRUCTTYPE_POSITIVE_POS_MOD
   },
   {
      .x =  66,
      .y = 120,
      .bPosition = B_POSITION_PLAYER_RIGHT,
      .type = HAILSTRUCTTYPE_POSITIVE_POS_MOD
   },
   {
      .x = 182,
      .y = 120,
      .bPosition = B_POSITION_OPPONENT_RIGHT,
      .type = HAILSTRUCTTYPE_NEGATIVE_POS_MOD
   },
   {
      .x =  60,
      .y = 120,
      .bPosition = B_POSITION_PLAYER_LEFT,
      .type = HAILSTRUCTTYPE_FIXED_POSITION
   },
   {
      .x = 214,
      .y = 120,
      .bPosition = B_POSITION_OPPONENT_LEFT,
      .type = HAILSTRUCTTYPE_NEGATIVE_POS_MOD
   },
   {
      .x = 113,
      .y = 120,
      .bPosition = B_POSITION_PLAYER_LEFT,
      .type = HAILSTRUCTTYPE_POSITIVE_POS_MOD
   },
   {
      .x = 210,
      .y = 120,
      .bPosition = B_POSITION_OPPONENT_RIGHT,
      .type = HAILSTRUCTTYPE_POSITIVE_POS_MOD
   },
   {
      .x =  38,
      .y = 120,
      .bPosition = B_POSITION_PLAYER_RIGHT,
      .type = HAILSTRUCTTYPE_NEGATIVE_POS_MOD
   },
};

static void TaskHandler(u8 taskId);

// task vars
#define tState             data[1]
#define tTimer             data[2]
#define tSpriteCount       data[3]
#define tHailTemplateIndex data[4]
#define tHailAffineAnimNum data[5]
#define tHailSpawnTimer    data[6]
enum {
   TASKSTATE_LOAD_GFX,
   TASKSTATE_DARKEN_BACKGROUND,
   TASKSTATE_WAITING,
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
static void SpriteAnim_FallingHail(struct Sprite*);

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
   .callback    = SpriteAnim_FallingHail,
};

static const struct CompressedSpriteSheet sHailParticleSpriteSheet = {
   gBattleAnimSpriteGfx_Hail, 0x0080, ANIM_TAG_HAIL
};
static const struct CompressedSpritePalette sHailParticleSpritePalette = {
   gBattleAnimSpritePal_Hail, ANIM_TAG_HAIL
};
// Hail sprite END

// Ice crystals START
#if ENABLE_HAIL_IMPACT_VFX
   static void SpriteAnim_HailImpactVFX(struct Sprite*);

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
   
   static const union AnimCmd sAnim_IceCrystalLarge[] = {
      ANIMCMD_FRAME(4, 1),
      ANIMCMD_END,
   };
   static const union AnimCmd *const sAnims_IceCrystalLarge[] = {
      sAnim_IceCrystalLarge,
   };

   static const struct SpriteTemplate sIceCrystalHitLargeSpriteTemplate = {
      .tileTag     = ANIM_TAG_ICE_CRYSTALS,
      .paletteTag  = ANIM_TAG_ICE_CRYSTALS,
      .oam         = &sOamData_AffineNormal_ObjBlend_8x16,
      .anims       = sAnims_IceCrystalLarge,
      .images      = NULL,
      .affineAnims = sAffineAnims_IceCrystalHit,
      .callback    = SpriteAnim_HailImpactVFX,
   };

   static const struct CompressedSpriteSheet sIceCrystalsSpriteSheet = {
      gBattleAnimSpriteGfx_IceCrystals, 0x01c0, ANIM_TAG_ICE_CRYSTALS
   };
   static const struct CompressedSpritePalette sIceCrystalsSpritePalette = {
      gBattleAnimSpritePal_IceCrystals, ANIM_TAG_ICE_CRYSTALS
   };
#endif
// Ice crystals END

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

static bool8 GenerateHailParticle(u8 hailStructId, u8 affineAnimNum);

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
         ++task->tState;
         break;
      case TASKSTATE_DARKEN_BACKGROUND:
         if (TaskHelper_BlendColors(taskId)) {
            // TODO: `loopsewithpan SE_M_HAIL, 0, 8, 10` instead of this
            PlaySE12WithPanning(SE_M_HAIL, SOUND_PAN_ATTACKER);
            
            DebugPrintf("[Battle Ambient Weather][Hail] Advancing to TASKSTATE_WAITING.");
            ++task->tState;
         }
         break;
      case TASKSTATE_WAITING:
         ++task->tTimer;
         if (task->tTimer > FALLING_HAIL_INITIAL_DELAY) {
            task->tTimer = 0;
            task->tHailAffineAnimNum = 0;
            ++task->tState;
            //DebugPrintf("[Battle Ambient Weather][Hail] Advancing to TASKSTATE_LOOPING.");
            break;
         }
         break;
      case TASKSTATE_LOOPING:
         if (task->tTimer == 0) {
            if (GenerateHailParticle(task->tHailTemplateIndex, task->tHailAffineAnimNum)) {
               ++task->tSpriteCount;
            }
            if (++task->tHailAffineAnimNum == ARRAY_COUNT(sAffineAnims_HailParticle)) {
               if (++task->tHailTemplateIndex == ARRAY_COUNT(sHailCoordData)) {
                  task->tHailTemplateIndex = 0;
               }
               --task->tState;
               //DebugPrintf("[Battle Ambient Weather][Hail] Rewinding to TASKSTATE_WAITING.");
            } else {
               task->tTimer = 1;
            }
         } else {
            --task->tTimer;
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

#define HAIL_VELOCITY_X 4
#define HAIL_VELOCITY_Y 8

// Sprite variables (falling hail)
#define sSpawnImpactEffect data[0]
#define sHailTargetX       data[3]
#define sHailTargetY       data[4]
#define sAffineAnimNum     data[5]
#define sOwnerTaskId       data[6]
#define sOwnerTaskSpriteCountField data[7]

static bool8 GenerateHailParticle(u8 hailStructId, u8 affineAnimNum) {
   s16 battlerX;
   s16 battlerY;
   bool8 shouldSpawnImpactEffect = FALSE;
   
   s8 type = sHailCoordData[hailStructId].type;
   if (type != HAILSTRUCTTYPE_FIXED_POSITION) {
      u8 id = GetBattlerAtPosition(sHailCoordData[hailStructId].bPosition);
      if (IsBattlerSpriteVisible(id)) {
         shouldSpawnImpactEffect = TRUE;
         battlerX = GetBattlerSpriteCoord(id, BATTLER_COORD_X_2);
         battlerY = GetBattlerSpriteCoord(id, BATTLER_COORD_Y_PIC_OFFSET);
         switch (type) {
            case HAILSTRUCTTYPE_NEGATIVE_POS_MOD:
               battlerX -= GetBattlerSpriteCoordAttr(id, BATTLER_COORD_ATTR_WIDTH) / 6;
               battlerY -= GetBattlerSpriteCoordAttr(id, BATTLER_COORD_ATTR_HEIGHT) / 6;
               break;
            case HAILSTRUCTTYPE_POSITIVE_POS_MOD:
               battlerX += GetBattlerSpriteCoordAttr(id, BATTLER_COORD_ATTR_WIDTH) / 6;
               battlerY += GetBattlerSpriteCoordAttr(id, BATTLER_COORD_ATTR_HEIGHT) / 6;
               break;
         }
      } else {
         battlerX = sHailCoordData[hailStructId].x;
         battlerY = sHailCoordData[hailStructId].y;
      }
   } else {
      battlerX = sHailCoordData[hailStructId].x;
      battlerY = sHailCoordData[hailStructId].y;
   }
   s16 spriteX = battlerX - ((battlerY + 8) / 2);
   
   u8 id = CreateSprite(&sHailParticleSpriteTemplate, spriteX, -8, 18);
   if (id == MAX_SPRITES) {
      return FALSE;
   }
   StartSpriteAffineAnim(&gSprites[id], affineAnimNum);
   gSprites[id].sSpawnImpactEffect = shouldSpawnImpactEffect;
   gSprites[id].sHailTargetX       = battlerX;
   gSprites[id].sHailTargetY       = battlerY;
   gSprites[id].sAffineAnimNum     = affineAnimNum;
   return TRUE;
}

static void SpriteAnim_FallingHail(struct Sprite* sprite) {
   if (gAmbientWeatherTaskId == TASK_NONE) {
      DestroySprite(sprite);
      return;
   }
   sprite->x += HAIL_VELOCITY_X;
   sprite->y += HAIL_VELOCITY_Y;

   if (sprite->x < sprite->sHailTargetX && sprite->y < sprite->sHailTargetY)
      return;

   #if ENABLE_HAIL_IMPACT_VFX
   if (sprite->sSpawnImpactEffect == 1 && sprite->sAffineAnimNum == 0) {
      //
      // Spawn an impact effect.
      //
      u8 spriteId = CreateSprite(
         &sIceCrystalHitLargeSpriteTemplate,
         sprite->sHailTargetX,
         sprite->sHailTargetY,
         sprite->subpriority
      );
   } else {
   #endif
      gTasks[gAmbientWeatherTaskId].tSpriteCount--;
   #if ENABLE_HAIL_IMPACT_VFX
   }
   #endif
   FreeOamMatrix(sprite->oam.matrixNum);
   DestroySprite(sprite);
}
#if ENABLE_HAIL_IMPACT_VFX
static void SpriteAnim_HailImpactVFX(struct Sprite *sprite) {
   if (gAmbientWeatherTaskId == TASK_NONE) {
      DestroySprite(sprite);
      return;
   }
   if (++sprite->data[0] == HAIL_IMPACT_VFX_LIFESPAN) {
      gTasks[gAmbientWeatherTaskId].tSpriteCount--;
      FreeOamMatrix(sprite->oam.matrixNum);
      DestroySprite(sprite);
   }
}
#endif