#include "global.h"
#include "constants/event_object_movement.h"
#include "constants/field_effects.h"
#include "constants/songs.h"
#include "event_object_movement.h"
#include "field_effect.h" // MultiplyPaletteRGBComponents
#include "field_effect_helpers.h"
#include "field_weather.h" // UpdateSpritePaletteWithWeather
#include "fldeff.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "script_movement.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "util.h"

#include "gba/isagbprint.h"

//
// UTIL
//

#define subsprite_table(ptr) {.subsprites = ptr, .subspriteCount = (sizeof ptr) / (sizeof(struct Subsprite))}

// These are originally defined in a place we can't #include from here.
static const u16 gFieldEffectObjectPalette0[] = INCBIN_U16("graphics/field_effects/palettes/general_0.gbapal");
static const struct SpritePalette gSpritePalette_GeneralFieldEffect0 = {gFieldEffectObjectPalette0, FLDEFF_PAL_TAG_GENERAL_0};

static const union AnimCmd sAnim_Static[] = {
   ANIMCMD_FRAME(.imageValue = 0, .duration = 1),
   ANIMCMD_JUMP(0)
};
static const union AnimCmd sAnim_Flicker[] = {
   ANIMCMD_FRAME(.imageValue = 0, .duration = 16),
   ANIMCMD_FRAME(.imageValue = 1, .duration = 16),
   ANIMCMD_FRAME(.imageValue = 0, .duration = 16),
   ANIMCMD_FRAME(.imageValue = 1, .duration = 16),
   ANIMCMD_FRAME(.imageValue = 0, .duration = 16),
   ANIMCMD_FRAME(.imageValue = 1, .duration = 16),
   ANIMCMD_FRAME(.imageValue = 0, .duration = 16),
   ANIMCMD_FRAME(.imageValue = 1, .duration = 16),
   ANIMCMD_END
};

static const struct OamData sOam_8x8 = {
   .y = 0,
   .affineMode = ST_OAM_AFFINE_OFF,
   .objMode = ST_OAM_OBJ_NORMAL,
   .bpp = ST_OAM_4BPP,
   .shape = SPRITE_SHAPE(8x8),
   .x = 0,
   .size = SPRITE_SIZE(8x8),
   .tileNum = 0,
   .priority = 0,
   .paletteNum = 0,
};
static const struct OamData sOam_16x16 = {
   .y = 0,
   .affineMode = ST_OAM_AFFINE_OFF,
   .objMode = ST_OAM_OBJ_NORMAL,
   .bpp = ST_OAM_4BPP,
   .shape = SPRITE_SHAPE(16x16),
   .x = 0,
   .size = SPRITE_SIZE(16x16),
   .tileNum = 0,
   .priority = 0,
   .paletteNum = 0,
};

static void FieldEffectFreeGraphicsResources(struct Sprite *sprite) {
   u16 sheetTileStart = sprite->sheetTileStart;
   u32 paletteNum = sprite->oam.paletteNum;
   DestroySprite(sprite);
   FieldEffectFreeTilesIfUnused(sheetTileStart);
   FieldEffectFreePaletteIfUnused(paletteNum);
}



//

#define FIRST_POKEBALL_X 93
#define FIRST_POKEBALL_Y 36

#define MONITOR_X 124
#define MONITOR_Y  24

// We want the PokeCenter nurse to insert Poke Balls two at a time. 
// We therefore shorten the delay on every other ball, as if she's 
// got a Poke Ball in each hand and is inserting them at very nearly 
// the same time.
#define POKECENTER_HEAL_DELAY_ON_PRIMARY_BALL 25
#define POKECENTER_HEAL_DELAY_ON_OFFHAND_BALL 9
#define POKECENTER_HEAL_DELAY_BETWEEN_CHANGE_DIR 7
#define POKECENTER_HEAL_DELAY_AFTER_LAST_BALL 24

static const u32 sPokeballGlow_Gfx[] = INCBIN_U32("graphics/field_effects/pics/pokeball_glow.4bpp");
static const u16 sPokeballGlow_Pal[16] = INCBIN_U16("graphics/field_effects/palettes/pokeball_glow.gbapal");

const struct SpritePalette sSpritePalette_PokeballGlow = {
   .data = sPokeballGlow_Pal,
   .tag  = FLDEFF_PAL_TAG_POKEBALL_GLOW
};


// Flicker on and off, for the Pokéballs / monitors during the PokéCenter heal effect
static const union AnimCmd *const sAnims_Flicker[] = {
   sAnim_Static,
   sAnim_Flicker
};

enum {
   TASK_STAGE_SETUP,
   TASK_STAGE_PLACE_POKEMON,
   TASK_STAGE_BLINKY,
   TASK_STAGE_FINISH,
};

// Task data for Task_PokecenterHeal
#define tState            data[0]
#define tNumMons          data[1]
#define tBallSpriteId     data[2]
#define tMonitorSpriteId  data[3]
#define tNurseEventID     data[4]
#define tNurseAction      data[5]
#define tNurseActionTimer data[6]
#define tSpawnedBallCount data[7]
#define tNurseActionStart data[8]
#define tNurseFacingMachine data[9]

// Sprite data for SpriteCB_PokeballGlowEffect
#define sState       data[0]
#define sTimer       data[1]
#define sCounter     data[2]
#define sInitialNumMons data[4]
#define sSpriteId    data[7]

// Sprite data for SpriteCB_PokeballGlow
#define sEffectSpriteId data[0]

static void Task_PokecenterHeal(u8 taskId);
static u8 CreateGlowingPokeballsEffect(void);
static u8 CreatePokecenterMonitorSprite(s16 x, s16 y);
static void CreatePokeballSprite(u8 nth_ball, u8 effect_sprite_id);

bool8 FldEff_PokecenterHeal(void) {
   LoadSpritePalette(&sSpritePalette_PokeballGlow);
   
   LoadSpritePalette(&gSpritePalette_GeneralFieldEffect0);
   UpdateSpritePaletteWithWeather(IndexOfSpritePaletteTag(gSpritePalette_GeneralFieldEffect0.tag));
   
   struct Task *task;

   DebugPrintf("[PokeCenter Nurse Field Effect] Spinning up task...");
   task = &gTasks[CreateTask(Task_PokecenterHeal, 0xff)];
   {
      u8 count = 0;
      for(int i = 0; i < PARTY_SIZE; ++i) {
         const struct Pokemon* mon = &gPlayerParty[i];
         if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
            continue;
         if (GetMonData(mon, MON_DATA_IS_EGG))
            continue;
         ++count;
      }
      task->tNumMons = count;
   }
   task->tNurseEventID = gFieldEffectArguments[0];
   return FALSE;
}

// returns TRUE if complete.
static void Action_FaceMachine_Start(struct Task*);
static bool8 Action_FaceMachine_ExtraWait(struct Task*);
static void Action_PlacePokemon(struct Task*);
static void Action_TakePokemon_Start(struct Task*);
static bool8 Action_TakePokemon_ExtraWait(struct Task*);

struct Action {
   void(*on_start)(struct Task*);
   bool8(*extra_wait)(struct Task*); // if non-NULL, return FALSE while waiting and TRUE when done
   void(*on_end)(struct Task*);
   u8 duration; // must be at least 1
};

// Individual actions taken by the nurse.
static const struct Action sActions[] = {
   {  // Facing the machine.
      .on_start   = Action_FaceMachine_Start,
      .extra_wait = Action_FaceMachine_ExtraWait,
      .on_end     = NULL,
      .duration   = 1,
   },
   {  // Placing a Poke Ball with the dominant hand.
      .on_end   = Action_PlacePokemon,
      .duration = 15,
   },
   {  // Placing a Poke Ball with the offhand.
      .on_end   = Action_PlacePokemon,
      .duration = 5,
   },
   {  // Facing the player, to take two more Poke Balls.
      .on_start   = Action_TakePokemon_Start,
      .extra_wait = Action_TakePokemon_ExtraWait,
      .on_end     = NULL,
      .duration   = 12,
   },
};

static void Task_PokecenterHeal(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   switch (task->tState) {
      case TASK_STAGE_SETUP:
         task->tBallSpriteId    = CreateGlowingPokeballsEffect();
         task->tMonitorSpriteId = CreatePokecenterMonitorSprite(MONITOR_X, MONITOR_Y);
         
         ++task->tState;
         task->tNurseAction        = 0;
         task->tNurseActionStart   = TRUE;
         task->tNurseActionTimer   = sActions[0].duration;
         task->tNurseFacingMachine = FALSE;
         DebugPrintf("[PokeCenter Nurse Field Effect] [Task] Advancing to state %u...", task->tState);
         break;
      case TASK_STAGE_PLACE_POKEMON:
         {
            const struct Action* action = &sActions[task->tNurseAction];
            if (task->tNurseActionStart) {
               task->tNurseActionStart = FALSE;
               if (action->on_start)
                  (action->on_start)(task);
               --task->tNurseActionTimer;
            } else if (task->tNurseActionTimer > 1) {
               bool8 advance = TRUE;
               if (action->extra_wait) {
                  //
                  // Some actions wait to wait on an external process before their 
                  // timers start ticking down. This includes actions for changing 
                  // the nurse's facing direction.
                  //
                  advance = (action->extra_wait)(task);
               }
               if (advance) {
                  --task->tNurseActionTimer;
               }
            } else {
               if (action->on_end)
                  (action->on_end)(task);
               //
               // Advance to the next action.
               //
               task->tNurseAction      = (task->tNurseAction + 1) % ARRAY_COUNT(sActions);
               task->tNurseActionStart = TRUE;
               task->tNurseActionTimer = sActions[task->tNurseAction].duration;
               if (task->tSpawnedBallCount >= task->tNumMons) {
                  //
                  // All Poke Balls have been loaded into the machine.
                  //
                  if (!task->tNurseFacingMachine) {
                     //
                     // Ensure that the nurse is facing the machine once she actually 
                     // starts operating it and healing the player's party: jump back 
                     // to the "face machine" action. We'll return here afterward and 
                     // then take the "is facing machine" branch.
                     //
                     task->tNurseAction      = 0;
                     task->tNurseActionTimer = sActions[task->tNurseAction].duration;
                  } else {
                     //
                     // All Poke Balls have been loaded into the machine. Delay the 
                     // start of the healing animation, and then advance to it.
                     //
                     ++task->tState;
                     ++gSprites[task->tBallSpriteId].sState;
                     gSprites[task->tBallSpriteId].sTimer = 24;
                     DebugPrintf("[PokeCenter Nurse Field Effect] [Task] Advancing to state %u...", task->tState);
                  }
               }
            }
         }
         break;
      case TASK_STAGE_BLINKY:
         //
         // Handle ball flashing.
         //
         if (gSprites[task->tBallSpriteId].sState > 4) {
            task->tState++;
            DebugPrintf("[PokeCenter Nurse Field Effect] [Task] Advancing to state %u...", task->tState);
         }
         break;
      case TASK_STAGE_FINISH:
         //
         // Wait for sound and end.
         //
         if (gSprites[task->tBallSpriteId].sState > 6) {
            DebugPrintf("[PokeCenter Nurse Field Effect] [Task] Done. Running teardown...");
            DestroySprite(&gSprites[task->tBallSpriteId]);
            FieldEffectActiveListRemove(FLDEFF_POKECENTER_HEAL);
            DestroyTask(FindTaskIdByFunc(Task_PokecenterHeal));
         }
         break;
   }
   
}

static struct ObjectEvent* GetNurse(struct Task* task) {
   return &gObjectEvents[task->tNurseEventID];
}

static void Action_FaceMachine_Start(struct Task* task) {
   DebugPrintf("[PokeCenter Nurse Field Effect] [Action: Face Machine] Start.");
   ObjectEventSetHeldMovement(GetNurse(task), MOVEMENT_ACTION_WALK_IN_PLACE_FASTER_LEFT);
   task->tNurseFacingMachine = TRUE;
}
static bool8 Action_FaceMachine_ExtraWait(struct Task* task) {
   struct ObjectEvent* nurse = GetNurse(task);
   return !ObjectEventIsHeldMovementActive(nurse);
}

static void Action_PlacePokemon(struct Task* task) {
   DebugPrintf("[PokeCenter Nurse Field Effect] [Action: Place Pokemon] End.");
   struct Sprite* sprite = &gSprites[task->tBallSpriteId];
   CreatePokeballSprite(task->tSpawnedBallCount, sprite->sSpriteId);
   ++task->tSpawnedBallCount;
   PlaySE(SE_BALL);
}

static void Action_TakePokemon_Start(struct Task* task) {
   DebugPrintf("[PokeCenter Nurse Field Effect] [Action: Take Pokemon] Start.");
   ObjectEventSetHeldMovement(GetNurse(task), MOVEMENT_ACTION_WALK_IN_PLACE_FASTER_DOWN);
   task->tNurseFacingMachine = FALSE;
}
static bool8 Action_TakePokemon_ExtraWait(struct Task* task) {
   struct ObjectEvent* nurse = GetNurse(task);
   return !ObjectEventIsHeldMovementActive(nurse);
}

//
// SINGLE POKEBALL SPRITE
//

static const struct SpriteFrameImage sPicTable_PokeballGlow[] = {
   obj_frame_tiles(sPokeballGlow_Gfx)
};

static void SpriteCB_PokeballGlow(struct Sprite *sprite) {
   if (gSprites[sprite->sEffectSpriteId].sState > 4) {
      DebugPrintf("[PokeCenter Nurse Field Effect] [Single Glow Sprite?] Dying.");
      FieldEffectFreeGraphicsResources(sprite);
   }
}

static const struct SpriteTemplate sSpriteTemplate_PokeballGlow = {
   .tileTag     = TAG_NONE,
   .paletteTag  = FLDEFF_PAL_TAG_POKEBALL_GLOW,
   .oam         = &sOam_8x8,
   .anims       = sAnims_Flicker,
   .images      = sPicTable_PokeballGlow,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback    = SpriteCB_PokeballGlow
};

static const struct Coords16 sPokeballCoordOffsets[PARTY_SIZE] = {
   {.x = 0, .y = 0},
   {.x = 6, .y = 0},
   {.x = 0, .y = 4},
   {.x = 6, .y = 4},
   {.x = 0, .y = 8},
   {.x = 6, .y = 8}
};
//
static void CreatePokeballSprite(u8 nth_ball, u8 effect_sprite_id) {
   u8 spriteID = CreateSpriteAtEnd(
      &sSpriteTemplate_PokeballGlow,
      sPokeballCoordOffsets[nth_ball].x + FIRST_POKEBALL_X,
      sPokeballCoordOffsets[nth_ball].y + FIRST_POKEBALL_Y,
      0
   );
   gSprites[spriteID].oam.priority    = 2;
   gSprites[spriteID].sEffectSpriteId = effect_sprite_id;
}

//
// POKEBALL GLOW SPRITE
//

static void SpriteCB_PokeballGlowEffect(struct Sprite *sprite);

static u8 CreateGlowingPokeballsEffect(void) {
   struct Sprite *sprite;
   u8 spriteID = CreateInvisibleSprite(SpriteCB_PokeballGlowEffect);
   sprite = &gSprites[spriteID];
   sprite->sSpriteId = spriteID;
   return spriteID;
}

static void PokeballGlowEffect_TryPlaySe(struct Sprite *sprite);
static void PokeballGlowEffect_Flash1(struct Sprite *sprite);
static void PokeballGlowEffect_Flash2(struct Sprite *sprite);
static void PokeballGlowEffect_WaitAfterFlash(struct Sprite *sprite);
static void PokeballGlowEffect_Dummy(struct Sprite *sprite);
static void PokeballGlowEffect_WaitForSound(struct Sprite *sprite);
static void PokeballGlowEffect_Idle(struct Sprite *sprite);
//
static void (*const sPokeballGlowEffectFuncs[])(struct Sprite *) = {
   PokeballGlowEffect_Idle,
   PokeballGlowEffect_TryPlaySe,
   PokeballGlowEffect_Flash1,
   PokeballGlowEffect_Flash2,
   PokeballGlowEffect_WaitAfterFlash,
   PokeballGlowEffect_Dummy,
   PokeballGlowEffect_WaitForSound,
   PokeballGlowEffect_Idle
};

static void SpriteCB_PokeballGlowEffect(struct Sprite *sprite) {
   sPokeballGlowEffectFuncs[sprite->sState](sprite);
}

static void PokeballGlowEffect_TryPlaySe(struct Sprite *sprite) {
   if ((--sprite->sTimer) == 0) {
      DebugPrintf("[PokeCenter Nurse Field Effect] [Glow Sprite] Playing healing fanfare...");
      sprite->sState++;
      sprite->sTimer = 8;
      sprite->sCounter = 0;
      sprite->data[3] = 0;
      PlayFanfare(MUS_HEAL);
   }
}

static const u8 sPokeballGlowReds[]   = {16, 12, 8, 0};
static const u8 sPokeballGlowGreens[] = {16, 12, 8, 0};
static const u8 sPokeballGlowBlues[]  = { 0,  0, 0, 0};

static void PokeballGlowEffect_Flash1(struct Sprite *sprite) {
   if ((--sprite->sTimer) == 0) {
      sprite->sTimer = 8;
      sprite->sCounter++;
      sprite->sCounter &= 3;

      if (sprite->sCounter == 0)
         sprite->data[3]++;
   }
   
   u8 phase = (sprite->sCounter + 3) & 3;
   MultiplyInvertedPaletteRGBComponents(
      OBJ_PLTT_ID(IndexOfSpritePaletteTag(FLDEFF_PAL_TAG_POKEBALL_GLOW)) + 8,
      sPokeballGlowReds[phase],
      sPokeballGlowGreens[phase],
      sPokeballGlowBlues[phase]
   );
   phase = (sprite->sCounter + 2) & 3;
   MultiplyInvertedPaletteRGBComponents(
      OBJ_PLTT_ID(IndexOfSpritePaletteTag(FLDEFF_PAL_TAG_POKEBALL_GLOW)) + 6,
      sPokeballGlowReds[phase],
      sPokeballGlowGreens[phase],
      sPokeballGlowBlues[phase]
   );
   phase = (sprite->sCounter + 1) & 3;
   MultiplyInvertedPaletteRGBComponents(
      OBJ_PLTT_ID(IndexOfSpritePaletteTag(FLDEFF_PAL_TAG_POKEBALL_GLOW)) + 2,
      sPokeballGlowReds[phase],
      sPokeballGlowGreens[phase],
      sPokeballGlowBlues[phase]
   );
   phase = sprite->sCounter;
   MultiplyInvertedPaletteRGBComponents(
      OBJ_PLTT_ID(IndexOfSpritePaletteTag(FLDEFF_PAL_TAG_POKEBALL_GLOW)) + 5,
      sPokeballGlowReds[phase],
      sPokeballGlowGreens[phase],
      sPokeballGlowBlues[phase]
   );
   MultiplyInvertedPaletteRGBComponents(
      OBJ_PLTT_ID(IndexOfSpritePaletteTag(FLDEFF_PAL_TAG_POKEBALL_GLOW)) + 3,
      sPokeballGlowReds[phase],
      sPokeballGlowGreens[phase],
      sPokeballGlowBlues[phase]
   );
   
   if (sprite->data[3] > 2) {
      DebugPrintf("[PokeCenter Nurse Field Effect] [Glow Sprite] Flash 1 complete.");
      sprite->sState++;
      sprite->sTimer = 8;
      sprite->sCounter = 0;
   }
}

static void PokeballGlowEffect_Flash2(struct Sprite *sprite) {
   if ((--sprite->sTimer) == 0) {
      sprite->sTimer = 8;
      sprite->sCounter++;
      sprite->sCounter &= 3;
      if (sprite->sCounter == 3) {
         DebugPrintf("[PokeCenter Nurse Field Effect] [Glow Sprite] Flash 2 complete.");
         sprite->sState++;
         sprite->sTimer = 30;
      }
   }
   
   u8 phase = sprite->sCounter;
   MultiplyInvertedPaletteRGBComponents(
      OBJ_PLTT_ID(IndexOfSpritePaletteTag(FLDEFF_PAL_TAG_POKEBALL_GLOW)) + 8,
      sPokeballGlowReds[phase],
      sPokeballGlowGreens[phase],
      sPokeballGlowBlues[phase]
   );
   MultiplyInvertedPaletteRGBComponents(
      OBJ_PLTT_ID(IndexOfSpritePaletteTag(FLDEFF_PAL_TAG_POKEBALL_GLOW)) + 6,
      sPokeballGlowReds[phase],
      sPokeballGlowGreens[phase],
      sPokeballGlowBlues[phase]
   );
   MultiplyInvertedPaletteRGBComponents(
      OBJ_PLTT_ID(IndexOfSpritePaletteTag(FLDEFF_PAL_TAG_POKEBALL_GLOW)) + 2,
      sPokeballGlowReds[phase],
      sPokeballGlowGreens[phase],
      sPokeballGlowBlues[phase]
   );
   MultiplyInvertedPaletteRGBComponents(
      OBJ_PLTT_ID(IndexOfSpritePaletteTag(FLDEFF_PAL_TAG_POKEBALL_GLOW)) + 5,
      sPokeballGlowReds[phase],
      sPokeballGlowGreens[phase],
      sPokeballGlowBlues[phase]
   );
   MultiplyInvertedPaletteRGBComponents(
      OBJ_PLTT_ID(IndexOfSpritePaletteTag(FLDEFF_PAL_TAG_POKEBALL_GLOW)) + 3,
      sPokeballGlowReds[phase],
      sPokeballGlowGreens[phase],
      sPokeballGlowBlues[phase]
   );
}

static void PokeballGlowEffect_WaitAfterFlash(struct Sprite *sprite) {
   if ((--sprite->sTimer) == 0) {
      DebugPrintf("[PokeCenter Nurse Field Effect] [Glow Sprite] Wait-after-flash complete.");
      sprite->sState++;
   }
}

static void PokeballGlowEffect_Dummy(struct Sprite *sprite) {
   DebugPrintf("[PokeCenter Nurse Field Effect] [Glow Sprite] Dummy phase complete.");
   sprite->sState++;
}

static void PokeballGlowEffect_WaitForSound(struct Sprite *sprite) {
   if (IsFanfareTaskInactive()) {
      DebugPrintf("[PokeCenter Nurse Field Effect] [Glow Sprite] Wait-for-fanfare complete.");
      sprite->sState++;
   }
}

static void PokeballGlowEffect_Idle(struct Sprite *sprite) {
   // Intentional no-op.
}

//
// MONITOR SPRITE
//

static const u32 sPokecenterMonitor0_Gfx[] = INCBIN_U32("graphics/field_effects/pics/pokecenter_monitor/0.4bpp");
static const u32 sPokecenterMonitor1_Gfx[] = INCBIN_U32("graphics/field_effects/pics/pokecenter_monitor/1.4bpp");

static void SpriteCB_PokecenterMonitor(struct Sprite *sprite);

static const struct SpriteFrameImage sPicTable_PokecenterMonitor[] = {
   obj_frame_tiles(sPokecenterMonitor0_Gfx),
   obj_frame_tiles(sPokecenterMonitor1_Gfx)
};
static const struct SpriteTemplate sSpriteTemplate_PokecenterMonitor = {
   .tileTag     = TAG_NONE,
   .paletteTag  = FLDEFF_PAL_TAG_GENERAL_0,
   .oam         = &sOam_16x16,
   .anims       = sAnims_Flicker,
   .images      = sPicTable_PokecenterMonitor,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback    = SpriteCB_PokecenterMonitor
};

/*
[0_][] <-1    24x16
[2 ][] <-3
   ^-- Origin
*/
static const struct Subsprite sSubsprites_PokecenterMonitor[] = {
   {
      .x = -12,
      .y =  -8,
      .shape = SPRITE_SHAPE(16x8),
      .size = SPRITE_SIZE(16x8),
      .tileOffset = 0,
      .priority = 2
   },
   {
      .x =  4,
      .y = -8,
      .shape = SPRITE_SHAPE(8x8),
      .size = SPRITE_SIZE(8x8),
      .tileOffset = 2,
      .priority = 2
   },
   {
      .x = -12,
      .y =   0,
      .shape = SPRITE_SHAPE(16x8),
      .size = SPRITE_SIZE(16x8),
      .tileOffset = 3,
      .priority = 2
   },
   {
      .x = 4,
      .y = 0,
      .shape = SPRITE_SHAPE(8x8),
      .size = SPRITE_SIZE(8x8),
      .tileOffset = 5,
      .priority = 2
   }
};

static const struct SubspriteTable sSubspriteTable_PokecenterMonitor = subsprite_table(sSubsprites_PokecenterMonitor);

static u8 CreatePokecenterMonitorSprite(s16 x, s16 y) {
   struct Sprite *sprite;
   u8 spriteId = CreateSpriteAtEnd(&sSpriteTemplate_PokecenterMonitor, x, y, 0);
   sprite = &gSprites[spriteId];
   sprite->oam.priority = 2;
   sprite->invisible = TRUE;
   SetSubspriteTables(sprite, &sSubspriteTable_PokecenterMonitor);
   return spriteId;
}

static void SpriteCB_PokecenterMonitor(struct Sprite *sprite) {
   if (sprite->data[0] != 0) {
      DebugPrintf("[PokeCenter Nurse Field Effect] [Monitor Sprite] Animation starting...");
      sprite->data[0] = 0;
      sprite->invisible = FALSE;
      StartSpriteAnim(sprite, 1);
   }
   if (sprite->animEnded) {
      DebugPrintf("[PokeCenter Nurse Field Effect] [Monitor Sprite] Animation ended.");
      FieldEffectFreeGraphicsResources(sprite);
   }
}