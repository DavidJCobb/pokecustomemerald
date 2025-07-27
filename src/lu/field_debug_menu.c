#include "lu/field_debug_menu.h"
#include "strings/field_debug_menu.h"

#include "global.h" // *sigh*
#include "gba/isagbprint.h"
#include "lu/c.h"

// UI
#include "constants/songs.h"
#include "bg.h"
#include "main.h"
#include "menu.h"
#include "sound.h"
#include "strings.h" // gText_SelectorArrow3
#include "text.h"
#include "window.h"

// Field UI
#include "event_object_lock.h" // ScriptUnfreezeObjectEvents
#include "event_object_movement.h" // FreezeObjectEvents
#include "field_player_avatar.h" // PlayerFreeze, StopPlayerAvatar // StartFishing
#include "script.h" // LockPlayerFieldControls, ScriptContext_SetupScript

// Data
#include "constants/global.h"
#include "constants/moves.h"
#include "data.h" // gMoveNames

// Menu actions
#include "constants/field_effects.h" // FLDEFF_...
#include "constants/items.h" // OLD_ROD and friends
#include "constants/species.h"
#include "constants/weather.h" // WEATHER_...
#include "battle_transition.h"
#include "bike.h" // GetOnOffBike
#include "event_data.h" // EnableNationalPokedex, FlagSet
#include "event_scripts.h" // EventScript_...
#include "field_control_avatar.h" // TrySetDiveWarp
#include "field_effect.h" // FieldEffectStart
#include "field_weather.h" // SetNextWeather
#include "fldeff.h" // SetUpFieldMove_RockSmash and friends
#include "global.fieldmap.h" // gPlayerAvatar, PLAYER_AVATAR_FLAG_...
#include "money.h" // GetMoney, SetMoney
#include "overworld.h" // CB2_ReturnToField, IsOverworldLinkActive
#include "pokedex.h"
#include "region_map.h" // CB2_OpenFlyMap
#include "string_util.h"
#include "wallclock.h" // CB2_StartWallClock
#include "lu/naming_screen.h"
#include "lu/pick_species_menu.h"
#include "lu/widgets/num_edit_modal.h"

EWRAM_DATA struct FieldDebugMenuState gFieldDebugMenuState = {0};

static void TaskHandler(u8 taskId);
static void PaintFieldDebugMenuActions(u8 taskId);
static void DestroyFieldDebugMenu(u8 taskId);

#define TILES_PER_ROW 2
#define ROW_COUNT     4

#define DEBUG_MENU_FONT FONT_NORMAL

static const struct WindowTemplate sWindowTemplate = {
   .bg          = 0,
   .tilemapLeft = 1,
   .tilemapTop  = 1,
   .width       = 14,
   .height      = (ROW_COUNT * TILES_PER_ROW),
   .paletteNum  = 15,
   .baseBlock   = 0x139,
};

enum {
   MENU_ACTION_DISABLE_TRAINER_LOS,
   MENU_ACTION_DISABLE_WILD_ENCOUNTERS,
   MENU_ACTION_ENABLE_NATIONAL_DEX,
   MENU_ACTION_FAST_TRAVEL,
   MENU_ACTION_USE_ANY_BIKE,
   MENU_ACTION_USE_ANY_FIELD_MOVE,
   MENU_ACTION_USE_ANY_FISHING_ROD,
   MENU_ACTION_VIEW_POKEDEX_ENTRY,
   MENU_ACTION_SET_MONEY,
   MENU_ACTION_SET_TIME,
   MENU_ACTION_SET_WEATHER,
   MENU_ACTION_TEST_BATTLE_TRANSITION,
   MENU_ACTION_TEST_VUI_NAMING_SCREEN,
   MENU_ACTION_WALK_THROUGH_WALLS,
   //
   NUM_MENU_ACTIONS
};

enum {
   MENU_ACTION_STATE_NORMAL,
   MENU_ACTION_STATE_ACTIVE,
   MENU_ACTION_STATE_DISABLED,
};

typedef void(*FieldDebugMenuActionHandler)(u8 taskId);
typedef u8(*FieldDebugMenuActionStateGetter)(void);

static u8   FieldDebugMenuActionStateGetter_DisableTrainerLOS(void);
static void FieldDebugMenuActionHandler_DisableTrainerLOS(u8 taskId);
static u8   FieldDebugMenuActionStateGetter_DisableWildEncounters(void);
static void FieldDebugMenuActionHandler_DisableWildEncounters(u8 taskId);
static u8   FieldDebugMenuActionStateGetter_EnableNationalDex(void);
static void FieldDebugMenuActionHandler_EnableNationalDex(u8 taskId);
static void FieldDebugMenuActionHandler_FastTravel(u8 taskId);
static void FieldDebugMenuActionHandler_SetMoney(u8 taskId);
static void FieldDebugMenuActionHandler_SetTime(u8 taskId);
static void FieldDebugMenuActionHandler_SetWeather(u8 taskId);
static void FieldDebugMenuActionHandler_BattleTransition(u8 taskId);
static void FieldDebugMenuActionHandler_TestVUINamingScreen(u8 taskId);
static void FieldDebugMenuActionHandler_UseAnyBike(u8 taskId);
static void FieldDebugMenuActionHandler_UseAnyFishingRod(u8 taskId);
static void FieldDebugMenuActionHandler_UseAnyFieldMove(u8 taskId);
static void FieldDebugMenuActionHandler_ViewPokedexEntry(u8 taskId);
static u8   FieldDebugMenuActionStateGetter_WalkThroughWalls(void);
static void FieldDebugMenuActionHandler_WalkThroughWalls(u8 taskId);

struct FieldDebugMenuAction {
   const u8*             label;
   FieldDebugMenuActionHandler     handler;
   FieldDebugMenuActionStateGetter state;
};
static const struct FieldDebugMenuAction sFieldDebugMenuActions[] = {
   [MENU_ACTION_DISABLE_TRAINER_LOS] = {
      .label   = gText_lu_FieldDebugMenu_DisableTrainerLOS,
      .handler = FieldDebugMenuActionHandler_DisableTrainerLOS,
      .state   = FieldDebugMenuActionStateGetter_DisableTrainerLOS,
   },
   [MENU_ACTION_DISABLE_WILD_ENCOUNTERS] = {
      .label   = gText_lu_FieldDebugMenu_DisableWildEncounters,
      .handler = FieldDebugMenuActionHandler_DisableWildEncounters,
      .state   = FieldDebugMenuActionStateGetter_DisableWildEncounters,
   },
   [MENU_ACTION_ENABLE_NATIONAL_DEX] = {
      .label   = gText_lu_FieldDebugMenu_EnableNationalDex,
      .handler = FieldDebugMenuActionHandler_EnableNationalDex,
   },
   [MENU_ACTION_FAST_TRAVEL] = {
      .label   = gText_lu_FieldDebugMenu_FastTravel,
      .handler = FieldDebugMenuActionHandler_FastTravel,
   },
   [MENU_ACTION_USE_ANY_BIKE] = {
      .label   = gText_lu_FieldDebugMenu_UseAnyBike,
      .handler = FieldDebugMenuActionHandler_UseAnyBike,
   },
   [MENU_ACTION_USE_ANY_FIELD_MOVE] = {
      .label   = gText_lu_FieldDebugMenu_UseAnyFieldMove,
      .handler = FieldDebugMenuActionHandler_UseAnyFieldMove,
   },
   [MENU_ACTION_USE_ANY_FISHING_ROD] = {
      .label   = gText_lu_FieldDebugMenu_UseAnyFishingRod,
      .handler = FieldDebugMenuActionHandler_UseAnyFishingRod,
   },
   [MENU_ACTION_VIEW_POKEDEX_ENTRY] = {
      .label   = gText_lu_FieldDebugMenu_ViewPokedexEntry,
      .handler = FieldDebugMenuActionHandler_ViewPokedexEntry,
   },
   [MENU_ACTION_SET_MONEY] = {
      .label   = gText_lu_FieldDebugMenu_SetMoney,
      .handler = FieldDebugMenuActionHandler_SetMoney,
   },
   [MENU_ACTION_SET_TIME] = {
      .label   = gText_lu_FieldDebugMenu_SetTime,
      .handler = FieldDebugMenuActionHandler_SetTime,
   },
   [MENU_ACTION_SET_WEATHER] = {
      .label   = gText_lu_FieldDebugMenu_SetWeather,
      .handler = FieldDebugMenuActionHandler_SetWeather,
   },
   [MENU_ACTION_TEST_BATTLE_TRANSITION] = {
      .label   = gText_lu_FieldDebugMenu_TestBattleTransition,
      .handler = FieldDebugMenuActionHandler_BattleTransition,
   },
   [MENU_ACTION_TEST_VUI_NAMING_SCREEN] = {
      .label   = gText_lu_FieldDebugMenu_TestVUINamingScreen,
      .handler = FieldDebugMenuActionHandler_TestVUINamingScreen,
   },
   [MENU_ACTION_WALK_THROUGH_WALLS] = {
      .label   = gText_lu_FieldDebugMenu_WalkThroughWalls,
      .handler = FieldDebugMenuActionHandler_WalkThroughWalls,
      .state   = FieldDebugMenuActionStateGetter_WalkThroughWalls,
   },
};

static void FieldDebugMenuActionHandler_Bike_Acro(u8 taskId);
static void FieldDebugMenuActionHandler_Bike_Mach(u8 taskId);

static const struct FieldDebugMenuAction sBikeActions[] = {
   {
      .label   = gText_lu_FieldDebugMenu_UseAnyBike_Acro,
      .handler = FieldDebugMenuActionHandler_Bike_Acro,
   },
   {
      .label   = gText_lu_FieldDebugMenu_UseAnyBike_Mach,
      .handler = FieldDebugMenuActionHandler_Bike_Mach,
   },
};

static void FieldDebugMenuActionHandler_UseAnyFishingRod_Old(u8 taskId);
static void FieldDebugMenuActionHandler_UseAnyFishingRod_Good(u8 taskId);
static void FieldDebugMenuActionHandler_UseAnyFishingRod_Super(u8 taskId);

static const struct FieldDebugMenuAction sFishingRodActions[] = {
   {
      .label   = gText_lu_FieldDebugMenu_UseAnyFishingRod_Old,
      .handler = FieldDebugMenuActionHandler_UseAnyFishingRod_Old,
   },
   {
      .label   = gText_lu_FieldDebugMenu_UseAnyFishingRod_Good,
      .handler = FieldDebugMenuActionHandler_UseAnyFishingRod_Good,
   },
   {
      .label   = gText_lu_FieldDebugMenu_UseAnyFishingRod_Super,
      .handler = FieldDebugMenuActionHandler_UseAnyFishingRod_Super,
   },
};

static void FieldDebugMenuActionHandler_FieldEffect_Cut(u8 taskId);
static void FieldDebugMenuActionHandler_FieldEffect_Dig(u8 taskId);
static void FieldDebugMenuActionHandler_FieldEffect_Dive(u8 taskId);
static u8 FieldDebugMenuActionStateGetter_FieldEffect_Dive(void);
static void FieldDebugMenuActionHandler_FieldEffect_Flash(u8 taskId);
static void FieldDebugMenuActionHandler_FieldEffect_RockSmash(u8 taskId);
static void FieldDebugMenuActionHandler_FieldEffect_SecretPower(u8 taskId);
static void FieldDebugMenuActionHandler_FieldEffect_Strength(u8 taskId);
static void FieldDebugMenuActionHandler_FieldEffect_Surf(u8 taskId);
static void FieldDebugMenuActionHandler_FieldEffect_SweetScent(u8 taskId);
static void FieldDebugMenuActionHandler_FieldEffect_Teleport(u8 taskId);
static void FieldDebugMenuActionHandler_FieldEffect_Waterfall(u8 taskId);

static const struct FieldDebugMenuAction sFieldEffectActions[] = {
   {
      .label   = gMoveNames[MOVE_CUT],
      .handler = FieldDebugMenuActionHandler_FieldEffect_Cut,
   },
   {
      .label   = gMoveNames[MOVE_DIG],
      .handler = FieldDebugMenuActionHandler_FieldEffect_Dig,
   },
   {
      .label   = gMoveNames[MOVE_DIVE],
      .handler = FieldDebugMenuActionHandler_FieldEffect_Dive,
      .state   = FieldDebugMenuActionStateGetter_FieldEffect_Dive,
   },
   {
      .label   = gMoveNames[MOVE_FLASH],
      .handler = FieldDebugMenuActionHandler_FieldEffect_Flash,
   },
   {
      .label   = gMoveNames[MOVE_ROCK_SMASH],
      .handler = FieldDebugMenuActionHandler_FieldEffect_RockSmash,
   },
   {
      .label   = gMoveNames[MOVE_SECRET_POWER],
      .handler = FieldDebugMenuActionHandler_FieldEffect_SecretPower,
      .state   = FieldDebugMenuActionStateGetter_FieldEffect_SecretPower,
   },
   {
      .label   = gMoveNames[MOVE_STRENGTH],
      .handler = FieldDebugMenuActionHandler_FieldEffect_Strength,
   },
   {
      .label   = gMoveNames[MOVE_SURF],
      .handler = FieldDebugMenuActionHandler_FieldEffect_Surf,
   },
   {
      .label   = gMoveNames[MOVE_SWEET_SCENT],
      .handler = FieldDebugMenuActionHandler_FieldEffect_SweetScent,
   },
   {
      .label   = gMoveNames[MOVE_TELEPORT],
      .handler = FieldDebugMenuActionHandler_FieldEffect_Teleport,
   },
   {
      .label   = gMoveNames[MOVE_WATERFALL],
      .handler = FieldDebugMenuActionHandler_FieldEffect_Waterfall,
   },
};

//
// Menu code
//

#define tSetupStage    data[0]
#define tCursorPos     data[1]
#define tWindowID      data[2]
#define tMenuPointerA  data[3]
#define tMenuPointerB  data[4]
#define tMenuItemCount data[5]
enum {
   TASK_SETUP_STAGE_GFX,
   TASK_SETUP_STAGE_LOCK,
   TASK_SETUP_STAGE_DONE,
};
#define MENU_TASK_SET_MENU(menu) \
   do { \
      SetWordTaskArg(taskId, 3, (u32)menu); \
      gTasks[taskId].tMenuItemCount = ARRAY_COUNT(menu); \
      gTasks[taskId].tCursorPos = 0; \
      DebugPrintf("[Field Debug Menu] Entering (sub)menu with %u items.", ARRAY_COUNT(menu)); \
   } while (0);

extern void OpenFieldDebugMenu(void) {
   if (gFieldDebugMenuState.menu_is_open) {
      DebugPrintf("[Field Debug Menu] Failed to open (already open).");
      return;
   }
   DebugPrintf("[Field Debug Menu] Opening...");
   
   u8 taskId = CreateTask(TaskHandler, 0);
   gTasks[taskId].tSetupStage = 0;
   gTasks[taskId].tCursorPos  = 0;
   
   MENU_TASK_SET_MENU(sFieldDebugMenuActions);
   
   gFieldDebugMenuState.menu_is_open = TRUE;
}

static void TaskHandler(u8 taskId) {
   struct Task *task = &gTasks[taskId];
   if (task->tSetupStage < TASK_SETUP_STAGE_DONE) {
      switch (task->tSetupStage) {
         case TASK_SETUP_STAGE_GFX:
            DebugPrintf("[Field Debug Menu] Setup: TASK_SETUP_STAGE_GFX");
            LoadMessageBoxAndBorderGfx();
            task->tWindowID = AddWindow(&sWindowTemplate);
            DrawStdWindowFrame(task->tWindowID, FALSE);
            PaintFieldDebugMenuActions(taskId);
            CopyWindowToVram(task->tWindowID, COPYWIN_MAP);
            break;
         case TASK_SETUP_STAGE_LOCK:
            DebugPrintf("[Field Debug Menu] Setup: TASK_SETUP_STAGE_LOCK");
            if (!IsOverworldLinkActive()) {
               FreezeObjectEvents();
               PlayerFreeze();
               StopPlayerAvatar();
            }
            LockPlayerFieldControls();
            break;
      }
      ++task->tSetupStage;
      return;
   }
   //
   // Handle input.
   //
   {
      s8 by = 0;
      if (JOY_REPEAT(DPAD_UP)) {
         by = -1;
      } else if (JOY_REPEAT(DPAD_DOWN)) {
         by = 1;
      }
      if (by) {
         PlaySE(SE_SELECT);
         
         if (by < 0) {
            if (task->tCursorPos == 0) {
               task->tCursorPos = task->tMenuItemCount - 1;
            } else {
               --task->tCursorPos;
            }
         } else {
            task->tCursorPos = (task->tCursorPos + 1) % task->tMenuItemCount;
         }
         
         PaintFieldDebugMenuActions(taskId);
         return;
      }
   }
   if (JOY_NEW(A_BUTTON)) {
      const struct FieldDebugMenuAction* actions = (const struct FieldDebugMenuAction*)GetWordTaskArg(taskId, 3);
      const struct FieldDebugMenuAction* action = &actions[task->tCursorPos];
      if (action->handler == NULL) {
         PlaySE(SE_FAILURE);
         return;
      }
      
      PlaySE(SE_SELECT);
      (action->handler)(taskId);
      if (gTasks[taskId].isActive) {
         PaintFieldDebugMenuActions(taskId);
      } else {
         //
         // The menu action we just invoked chose to destroy the task.
         //
      }
      return;
   }
   if (JOY_NEW(B_BUTTON)) {
      PlaySE(SE_SELECT);
      
      const struct FieldDebugMenuAction* actions = (const struct FieldDebugMenuAction*)GetWordTaskArg(taskId, 3);
      if (actions == sFieldDebugMenuActions) {
         DestroyFieldDebugMenu(taskId);
      } else {
         MENU_TASK_SET_MENU(sFieldDebugMenuActions);
         PaintFieldDebugMenuActions(taskId);
      }
      
      return;
   }
}

static const u8 sTextColor_Normal[3]   = { 1, 2, 3 };
static const u8 sTextColor_Active[3]   = { 1, 4, 5 };
static const u8 sTextColor_Disabled[3] = { 1, 3, 1 };

static void PaintFieldDebugMenuActions(u8 taskId) {
   struct Task* task = &gTasks[taskId];
   
   const struct FieldDebugMenuAction* actions = (const struct FieldDebugMenuAction*)GetWordTaskArg(taskId, 3);
   const u16 menu_item_count = task->tMenuItemCount;
   
   u8 scroll_pos = task->tCursorPos;
   if (scroll_pos <= (ROW_COUNT / 2) || menu_item_count < ROW_COUNT) {
      scroll_pos = 0;
   } else {
      scroll_pos -= (ROW_COUNT / 2);
      if (scroll_pos + ROW_COUNT > menu_item_count) {
         scroll_pos = menu_item_count - ROW_COUNT;
      }
   }
   u8 screen_pos = task->tCursorPos - scroll_pos;
   
   FillWindowPixelBuffer(task->tWindowID, PIXEL_FILL(1));
   
   for(u8 i = 0; i < ROW_COUNT; ++i) {
      if (scroll_pos + i >= menu_item_count)
         break;
      const struct FieldDebugMenuAction* action = &actions[scroll_pos + i];
      
      u8 fontId = FONT_NORMAL;
      const u8* colors = sTextColor_Normal;
      if (action->state) {
         switch ((action->state)()) {
            case MENU_ACTION_STATE_NORMAL:
               break;
            case MENU_ACTION_STATE_ACTIVE:
               colors = sTextColor_Active;
               fontId = FONT_BOLD;
               break;
            case MENU_ACTION_STATE_DISABLED:
               colors = sTextColor_Disabled;
               break;
         }
      }
      AddTextPrinterParameterized3(
         task->tWindowID,
         fontId,
         8, // x
         i * TILES_PER_ROW * TILE_HEIGHT, // y
         colors,
         TEXT_SKIP_DRAW,
         action->label
      );
   }
   
   u8 width  = GetMenuCursorDimensionByFont(DEBUG_MENU_FONT, 0);
   u8 height = GetMenuCursorDimensionByFont(DEBUG_MENU_FONT, 1);
   AddTextPrinterParameterized(
      task->tWindowID,
      DEBUG_MENU_FONT,
      gText_SelectorArrow3,
      0,
      screen_pos * TILES_PER_ROW * TILE_HEIGHT,
      0,
      0
   );
   CopyWindowToVram(task->tWindowID, COPYWIN_GFX);
}

static void DestroyFieldDebugMenu(u8 taskId) {
   DebugPrintf("[Field Debug Menu] Closing...");
   struct Task *task = &gTasks[taskId];
   if (task->tWindowID != WINDOW_NONE) {
      ClearStdWindowAndFrame(task->tWindowID, TRUE);
      RemoveWindow(task->tWindowID);
      task->tWindowID = WINDOW_NONE;
   }
   ScriptUnfreezeObjectEvents();
   UnlockPlayerFieldControls();
   
   DestroyTask(taskId);
   gFieldDebugMenuState.menu_is_open = FALSE;
}

//
// Menu action handlers
//

static u8 FieldDebugMenuActionStateGetter_DisableTrainerLOS(void) {
   if (gFieldDebugMenuState.disable_trainer_line_of_sight) {
      return MENU_ACTION_STATE_ACTIVE;
   }
   return MENU_ACTION_STATE_NORMAL;
}
static void FieldDebugMenuActionHandler_DisableTrainerLOS(u8 taskId) {
   gFieldDebugMenuState.disable_trainer_line_of_sight = !gFieldDebugMenuState.disable_trainer_line_of_sight;
}
static u8 FieldDebugMenuActionStateGetter_DisableWildEncounters(void) {
   if (gFieldDebugMenuState.disable_wild_encounters) {
      return MENU_ACTION_STATE_ACTIVE;
   }
   return MENU_ACTION_STATE_NORMAL;
}
static void FieldDebugMenuActionHandler_DisableWildEncounters(u8 taskId) {
   gFieldDebugMenuState.disable_wild_encounters = !gFieldDebugMenuState.disable_wild_encounters;
}
static u8 FieldDebugMenuActionStateGetter_EnableNationalDex(void) {
   if (IsNationalPokedexEnabled()) {
      return MENU_ACTION_STATE_DISABLED;
   }
   return MENU_ACTION_STATE_NORMAL;
}
static void FieldDebugMenuActionHandler_EnableNationalDex(u8 taskId) {
   EnableNationalPokedex();
}
static void FieldDebugMenuActionHandler_FastTravel(u8 taskId) {
   gFieldDebugMenuState.allow_fast_travel_anywhere = TRUE;
   //
   // NOTE: This enables fast-travel anywhere for the rest of the session, 
   // i.e. using Fly normally allows you to fly anywhere. We need a way to 
   // clear this state bool, OR a way to avoid state bools in favor of 
   // passing parameters to the region map code.
   //
   DestroyFieldDebugMenu(taskId);
   SetMainCallback2(CB2_OpenFlyMap);
}

static void FieldDebugMenuActionHandler_SetMoney_callback(bool8 accepted, s32 value) {
   ScriptUnfreezeObjectEvents();
   UnlockPlayerFieldControls();
   if (accepted) {
      DebugPrintf("[Field Debug Menu] Setting money to %u...", value);
      SetMoney(&gSaveBlock1Ptr->money, value);
   }
}
static void FieldDebugMenuActionHandler_SetMoney(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   
   if (!IsOverworldLinkActive()) {
      FreezeObjectEvents();
      PlayerFreeze();
      StopPlayerAvatar();
   }
   LockPlayerFieldControls();
   
   struct LuNumEditModalInitParams params = {
      .min_value = 0,
      .max_value = 999999,
      .cur_value = GetMoney(&gSaveBlock1Ptr->money),
      .callback  = FieldDebugMenuActionHandler_SetMoney_callback,
      .use_task  = TRUE,
      .window    = {
         .bg_layer      = 0,
         .first_tile_id = 0x139,
         .palette_id    = 15,
         .x             = 2,
         .y             = 2,
      },
      .border = {
         .first_tile_id  = 0x130,
         .palette_id     = 14,
         .already_loaded = FALSE,
      },
      .text_colors = {
         .back   = 1,
         .text   = 2,
         .shadow = 3,
      },
      .sprite_tags = {
         .cursor = {
            0xFFFF,
            1234
         },
      },
   };
   FireAndForgetNumEditModal(&params);
}

static void FieldDebugMenuActionHandler_SetTime(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   SetMainCallback2(CB2_StartWallClock);
    gMain.savedCallback = CB2_ReturnToField;
}

static const u8 sRenamePlayerNamingScreenTitle[] = _("Change your name to…?");
//
static void FieldDebugMenuActionHandler_TestVUINamingScreen_Callback(const u8* buffer) {
   if (buffer && buffer[0] != EOS) {
      StringCopy(gSaveBlock2Ptr->playerName, buffer);
   }
   SetMainCallback2(CB2_ReturnToField);
}
static void FieldDebugMenuActionHandler_TestVUINamingScreen(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   
   struct LuNamingScreenParams params = {
      .callback      = FieldDebugMenuActionHandler_TestVUINamingScreen_Callback,
      .initial_value = gSaveBlock2Ptr->playerName,
      .max_length    = PLAYER_NAME_LENGTH,
      //
      .title         = sRenamePlayerNamingScreenTitle,
   };
   LuNamingScreen(&params);
}

static void FieldDebugMenuActionHandler_UseAnyBike(u8 taskId) {
   MENU_TASK_SET_MENU(sBikeActions);
}
static void FieldDebugMenuActionHandler_UseAnyFishingRod(u8 taskId) {
   MENU_TASK_SET_MENU(sFishingRodActions);
}
static void FieldDebugMenuActionHandler_UseAnyFieldMove(u8 taskId) {
   MENU_TASK_SET_MENU(sFieldEffectActions);
}
static u8 FieldDebugMenuActionStateGetter_WalkThroughWalls(void) {
   if (gFieldDebugMenuState.walk_through_walls) {
      return MENU_ACTION_STATE_ACTIVE;
   }
   return MENU_ACTION_STATE_NORMAL;
}
static void FieldDebugMenuActionHandler_WalkThroughWalls(u8 taskId) {
   gFieldDebugMenuState.walk_through_walls = !gFieldDebugMenuState.walk_through_walls;
}

//
// Bikes
//
static void FieldDebugMenuActionHandler_Bike_Acro(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   if (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_MACH_BIKE) {
      // If the player is already on the Mach Bike, then dismount, and then 
      // mount the Acro Bike.
      GetOnOffBike(PLAYER_AVATAR_FLAG_ACRO_BIKE);
   }
   GetOnOffBike(PLAYER_AVATAR_FLAG_ACRO_BIKE);
}
static void FieldDebugMenuActionHandler_Bike_Mach(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   if (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_ACRO_BIKE) {
      // If the player is already on the Acro Bike, then dismount, and then 
      // mount the Mach Bike.
      GetOnOffBike(PLAYER_AVATAR_FLAG_MACH_BIKE);
   }
   GetOnOffBike(PLAYER_AVATAR_FLAG_MACH_BIKE);
}

//
// Field effects
//
static void FieldDebugMenuActionHandler_FieldEffect_Cut(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   
   SetUpFieldMove_Cut();
   gFieldEffectArguments[0] = SPECIES_MISDREAVUS;
   (gPostMenuFieldCallback)();
}
static void FieldDebugMenuActionHandler_FieldEffect_Dig(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   
   Overworld_ResetStateAfterDigEscRope();
   FieldEffectStart(FLDEFF_USE_DIG);
   gFieldEffectArguments[0] = SPECIES_MISDREAVUS;
}
static void FieldDebugMenuActionHandler_FieldEffect_Dive(u8 taskId) {
   u8 dive_result_type = TrySetDiveWarp();
   if (dive_result_type == 0) {
      return;
   }
   DestroyFieldDebugMenu(taskId);
   if (dive_result_type == 1) {
      // The script checks for a Pokemon that can use the move, so it's not 
      // what we want here.
      //ScriptContext_SetupScript(EventScript_UseDiveUnderwater);
      
      gFieldEffectArguments[0] = SPECIES_MISDREAVUS;
      gFieldEffectArguments[0] = 1;
      FieldEffectStart(FLDEFF_USE_DIVE);
   } else {
      // The script checks for a Pokemon that can use the move, so it's not 
      // what we want here.
      //ScriptContext_SetupScript(EventScript_UseDive);
      
      gFieldEffectArguments[0] = SPECIES_MISDREAVUS;
      gFieldEffectArguments[0] = 1;
      FieldEffectStart(FLDEFF_USE_DIVE);
   }
}
static u8 FieldDebugMenuActionStateGetter_FieldEffect_Dive(void) {
   if (TrySetDiveWarp() == 0) {
      return MENU_ACTION_STATE_DISABLED;
   }
   return MENU_ACTION_STATE_NORMAL;
}
static void FieldDebugMenuActionHandler_FieldEffect_Flash(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   
   SetUpFieldMove_Flash();
   gFieldEffectArguments[0] = SPECIES_MISDREAVUS;
   auto callback = gPostMenuFieldCallback;
   gPostMenuFieldCallback = NULL;
   (callback)();
}
static void FieldDebugMenuActionHandler_FieldEffect_RockSmash(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   
   SetUpFieldMove_RockSmash();
   gFieldEffectArguments[0] = SPECIES_MISDREAVUS;
   auto callback = gPostMenuFieldCallback;
   gPostMenuFieldCallback = NULL;
   (callback)();
}
static void FieldDebugMenuActionHandler_FieldEffect_SecretPower(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   
   auto prior_result = gSpecialVar_Result;
   
   SetUpFieldMove_SecretPower();
   gFieldEffectArguments[0] = SPECIES_MISDREAVUS;
   auto callback = gPostMenuFieldCallback;
   gPostMenuFieldCallback = NULL;
   (callback)();
   
   gSpecialVar_Result     = prior_result;
}
static u8 FieldDebugMenuActionStateGetter_FieldEffect_SecretPower(void) {
   if (GetPlayerFacingDirection() != DIR_NORTH)
      return MENU_ACTION_STATE_DISABLED;
   
   bool8 has_secret_base;
   {
      auto prior_result = gSpecialVar_Result;
      CheckPlayerHasSecretBase();
      has_secret_base    = gSpecialVar_Result;
      gSpecialVar_Result = prior_result;
   }
   if (has_secret_base)
      return MENU_ACTION_STATE_DISABLED;
   
   GetXYCoordsOneStepInFrontOfPlayer(&gPlayerFacingPosition.x, &gPlayerFacingPosition.y);
   u8 mb = MapGridGetMetatileBehaviorAt(gPlayerFacingPosition.x, gPlayerFacingPosition.y);
   
   if (MetatileBehavior_IsSecretBaseCave(mb) == TRUE)
      return MENU_ACTION_STATE_NORMAL;
   if (MetatileBehavior_IsSecretBaseTree(mb) == TRUE)
      return MENU_ACTION_STATE_NORMAL;
   if (MetatileBehavior_IsSecretBaseShrub(mb) == TRUE)
      return MENU_ACTION_STATE_NORMAL;
   
   return MENU_ACTION_STATE_DISABLED;
}
static void FieldDebugMenuActionHandler_FieldEffect_Strength(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   
   FldEff_UseStrength();
   FlagSet(FLAG_SYS_USE_STRENGTH);
}
static void FieldDebugMenuActionHandler_FieldEffect_Surf(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   FieldEffectStart(FLDEFF_USE_SURF);
}
static void FieldDebugMenuActionHandler_FieldEffect_SweetScent(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   FieldEffectStart(FLDEFF_SWEET_SCENT);
   gFieldEffectArguments[0] = SPECIES_MISDREAVUS;
}
static void FieldDebugMenuActionHandler_FieldEffect_Teleport(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   
   Overworld_ResetStateAfterTeleport();
   FieldEffectStart(FLDEFF_USE_TELEPORT);
   gFieldEffectArguments[0] = SPECIES_MISDREAVUS;
}
static void FieldDebugMenuActionHandler_FieldEffect_Waterfall(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   FieldEffectStart(FLDEFF_USE_WATERFALL);
}

//
// Fishing Rods
//
static void FieldDebugMenuActionHandler_UseAnyFishingRod_Old(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   StartFishing(OLD_ROD);
}
static void FieldDebugMenuActionHandler_UseAnyFishingRod_Good(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   StartFishing(GOOD_ROD);
}
static void FieldDebugMenuActionHandler_UseAnyFishingRod_Super(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   StartFishing(SUPER_ROD);
}

//
// View Pokedex entry
//

static void FieldDebugMenuActionHandler_Task(u8 taskId) {
   u8 dex_task_id = gTasks[taskId].data[0];
   if (!gTasks[dex_task_id].isActive) {
      DestroyTask(taskId);
      SetMainCallback2(CB2_ReturnToField);
   }
}

static void FieldDebugMenuActionHandler_ViewPokedexEntry_Callback(u16 species) {
   u8 monitor_task_id = CreateTask(FieldDebugMenuActionHandler_Task, 50);
   if (monitor_task_id == TASK_NONE) {
      return;
   }
   u8 dex_task_id = DisplayCaughtMonDexPage(species, 0, 0);
   if (dex_task_id == TASK_NONE) {
      DestroyTask(monitor_task_id);
      return;
   }
   gTasks[monitor_task_id].data[0] = dex_task_id;
}

static const struct PickSpeciesMenuParams sPickSpeciesMenuParams = {
   .callback  = FieldDebugMenuActionHandler_ViewPokedexEntry_Callback,
   .zero_type = PICKSPECIESMENU_ZEROTYPE_NONE,
};
static void FieldDebugMenuActionHandler_ViewPokedexEntry(u8 taskId) {
   DestroyFieldDebugMenu(taskId);
   ShowPickSpeciesMenu(&sPickSpeciesMenuParams);
}

//
// Weather
//

#define WEATHER_HANDLER(name, constant) \
   static void FieldDebugMenuActionHandler_Weather_##name(u8 taskId) { \
      SetNextWeather(constant); \
   }

WEATHER_HANDLER(SunnyCloudy, WEATHER_SUNNY_CLOUDS)
WEATHER_HANDLER(Sunny, WEATHER_SUNNY)
WEATHER_HANDLER(Rain, WEATHER_RAIN)
WEATHER_HANDLER(Snow, WEATHER_SNOW)
WEATHER_HANDLER(Thunderstorm, WEATHER_RAIN_THUNDERSTORM)
WEATHER_HANDLER(Fog, WEATHER_FOG_HORIZONTAL)
WEATHER_HANDLER(Ashfall, WEATHER_VOLCANIC_ASH)
WEATHER_HANDLER(Sandstorm, WEATHER_SANDSTORM)
WEATHER_HANDLER(FogDiagonal, WEATHER_FOG_DIAGONAL)
WEATHER_HANDLER(Underwater, WEATHER_UNDERWATER)
WEATHER_HANDLER(Overcast, WEATHER_SHADE)
WEATHER_HANDLER(Drought, WEATHER_DROUGHT)
WEATHER_HANDLER(Downpour, WEATHER_DOWNPOUR)
WEATHER_HANDLER(UnderwaterBubbles, WEATHER_UNDERWATER_BUBBLES)
WEATHER_HANDLER(Abnormal, WEATHER_ABNORMAL)
WEATHER_HANDLER(Route119Cycle, WEATHER_ROUTE119_CYCLE)
WEATHER_HANDLER(Route123Cycle, WEATHER_ROUTE123_CYCLE)

#define WEATHER_MENU_ENTRY(name) \
   { \
      .label   = gText_lu_FieldDebugMenu_SetWeather_##name, \
      .handler = FieldDebugMenuActionHandler_Weather_##name, \
   }

static const struct FieldDebugMenuAction sWeatherActions[] = {
   WEATHER_MENU_ENTRY(SunnyCloudy),
   WEATHER_MENU_ENTRY(Sunny),
   WEATHER_MENU_ENTRY(Rain),
   WEATHER_MENU_ENTRY(Snow),
   WEATHER_MENU_ENTRY(Thunderstorm),
   WEATHER_MENU_ENTRY(Fog),
   WEATHER_MENU_ENTRY(Ashfall),
   WEATHER_MENU_ENTRY(Sandstorm),
   WEATHER_MENU_ENTRY(FogDiagonal),
   WEATHER_MENU_ENTRY(Underwater),
   WEATHER_MENU_ENTRY(Overcast),
   WEATHER_MENU_ENTRY(Drought),
   WEATHER_MENU_ENTRY(Downpour),
   WEATHER_MENU_ENTRY(UnderwaterBubbles),
   WEATHER_MENU_ENTRY(Abnormal),
   WEATHER_MENU_ENTRY(Route119Cycle),
   WEATHER_MENU_ENTRY(Route123Cycle),
};

static void FieldDebugMenuActionHandler_SetWeather(u8 taskId) {
   MENU_TASK_SET_MENU(sWeatherActions);
}

//
// Battle transitions
//

#define BATTLE_TRANSITION_HANDLER(name) \
   static void FieldDebugMenuActionHandler_BattleTransition_##name(u8 taskId) { \
      DestroyFieldDebugMenu(taskId); \
      BattleTransition_StartTest(B_TRANSITION_##name); \
   }

BATTLE_TRANSITION_HANDLER(BLUR)
BATTLE_TRANSITION_HANDLER(SWIRL)
BATTLE_TRANSITION_HANDLER(SHUFFLE)
BATTLE_TRANSITION_HANDLER(BIG_POKEBALL)
BATTLE_TRANSITION_HANDLER(POKEBALLS_TRAIL)
BATTLE_TRANSITION_HANDLER(CLOCKWISE_WIPE)
BATTLE_TRANSITION_HANDLER(RIPPLE)
BATTLE_TRANSITION_HANDLER(WAVE)
BATTLE_TRANSITION_HANDLER(SLICE)
BATTLE_TRANSITION_HANDLER(WHITE_BARS_FADE)
BATTLE_TRANSITION_HANDLER(GRID_SQUARES)
BATTLE_TRANSITION_HANDLER(ANGLED_WIPES)
BATTLE_TRANSITION_HANDLER(SIDNEY)
BATTLE_TRANSITION_HANDLER(PHOEBE)
BATTLE_TRANSITION_HANDLER(GLACIA)
BATTLE_TRANSITION_HANDLER(DRAKE)
BATTLE_TRANSITION_HANDLER(CHAMPION)
BATTLE_TRANSITION_HANDLER(AQUA)
BATTLE_TRANSITION_HANDLER(MAGMA)
BATTLE_TRANSITION_HANDLER(REGICE)
BATTLE_TRANSITION_HANDLER(REGISTEEL)
BATTLE_TRANSITION_HANDLER(REGIROCK)
BATTLE_TRANSITION_HANDLER(KYOGRE)
BATTLE_TRANSITION_HANDLER(GROUDON)
BATTLE_TRANSITION_HANDLER(RAYQUAZA)
BATTLE_TRANSITION_HANDLER(SHRED_SPLIT)
BATTLE_TRANSITION_HANDLER(BLACKHOLE)
BATTLE_TRANSITION_HANDLER(BLACKHOLE_PULSATE)
BATTLE_TRANSITION_HANDLER(RECTANGULAR_SPIRAL)
BATTLE_TRANSITION_HANDLER(FRONTIER_LOGO_WIGGLE)
BATTLE_TRANSITION_HANDLER(FRONTIER_LOGO_WAVE)
BATTLE_TRANSITION_HANDLER(FRONTIER_SQUARES)
BATTLE_TRANSITION_HANDLER(FRONTIER_SQUARES_SCROLL)
BATTLE_TRANSITION_HANDLER(FRONTIER_SQUARES_SPIRAL)
BATTLE_TRANSITION_HANDLER(FRONTIER_CIRCLES_MEET)
BATTLE_TRANSITION_HANDLER(FRONTIER_CIRCLES_CROSS)
BATTLE_TRANSITION_HANDLER(FRONTIER_CIRCLES_ASYMMETRIC_SPIRAL)
BATTLE_TRANSITION_HANDLER(FRONTIER_CIRCLES_SYMMETRIC_SPIRAL)
BATTLE_TRANSITION_HANDLER(FRONTIER_CIRCLES_MEET_IN_SEQ)
BATTLE_TRANSITION_HANDLER(FRONTIER_CIRCLES_CROSS_IN_SEQ)
BATTLE_TRANSITION_HANDLER(FRONTIER_CIRCLES_ASYMMETRIC_SPIRAL_IN_SEQ)
BATTLE_TRANSITION_HANDLER(FRONTIER_CIRCLES_SYMMETRIC_SPIRAL_IN_SEQ)

#define BATTLE_TRANSITION_MENU_ENTRY(name) \
   { \
      .label   = gText_lu_FieldDebugMenu_BattleTransition_##name, \
      .handler = FieldDebugMenuActionHandler_BattleTransition_##name, \
   }

static const struct FieldDebugMenuAction sBattleTransitionActions[] = {
   BATTLE_TRANSITION_MENU_ENTRY(BLUR),
   BATTLE_TRANSITION_MENU_ENTRY(SWIRL),
   BATTLE_TRANSITION_MENU_ENTRY(SHUFFLE),
   BATTLE_TRANSITION_MENU_ENTRY(BIG_POKEBALL),
   BATTLE_TRANSITION_MENU_ENTRY(POKEBALLS_TRAIL),
   BATTLE_TRANSITION_MENU_ENTRY(CLOCKWISE_WIPE),
   BATTLE_TRANSITION_MENU_ENTRY(RIPPLE),
   BATTLE_TRANSITION_MENU_ENTRY(WAVE),
   BATTLE_TRANSITION_MENU_ENTRY(SLICE),
   BATTLE_TRANSITION_MENU_ENTRY(WHITE_BARS_FADE),
   BATTLE_TRANSITION_MENU_ENTRY(GRID_SQUARES),
   BATTLE_TRANSITION_MENU_ENTRY(ANGLED_WIPES),
   BATTLE_TRANSITION_MENU_ENTRY(SIDNEY),
   BATTLE_TRANSITION_MENU_ENTRY(PHOEBE),
   BATTLE_TRANSITION_MENU_ENTRY(GLACIA),
   BATTLE_TRANSITION_MENU_ENTRY(DRAKE),
   BATTLE_TRANSITION_MENU_ENTRY(CHAMPION),
   BATTLE_TRANSITION_MENU_ENTRY(AQUA),
   BATTLE_TRANSITION_MENU_ENTRY(MAGMA),
   BATTLE_TRANSITION_MENU_ENTRY(REGICE),
   BATTLE_TRANSITION_MENU_ENTRY(REGISTEEL),
   BATTLE_TRANSITION_MENU_ENTRY(REGIROCK),
   BATTLE_TRANSITION_MENU_ENTRY(KYOGRE),
   BATTLE_TRANSITION_MENU_ENTRY(GROUDON),
   BATTLE_TRANSITION_MENU_ENTRY(RAYQUAZA),
   BATTLE_TRANSITION_MENU_ENTRY(SHRED_SPLIT),
   BATTLE_TRANSITION_MENU_ENTRY(BLACKHOLE),
   BATTLE_TRANSITION_MENU_ENTRY(BLACKHOLE_PULSATE),
   BATTLE_TRANSITION_MENU_ENTRY(RECTANGULAR_SPIRAL),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_LOGO_WIGGLE),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_LOGO_WAVE),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_SQUARES),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_SQUARES_SCROLL),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_SQUARES_SPIRAL),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_CIRCLES_MEET),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_CIRCLES_CROSS),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_CIRCLES_ASYMMETRIC_SPIRAL),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_CIRCLES_SYMMETRIC_SPIRAL),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_CIRCLES_MEET_IN_SEQ),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_CIRCLES_CROSS_IN_SEQ),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_CIRCLES_ASYMMETRIC_SPIRAL_IN_SEQ),
   BATTLE_TRANSITION_MENU_ENTRY(FRONTIER_CIRCLES_SYMMETRIC_SPIRAL_IN_SEQ),
};

static void FieldDebugMenuActionHandler_BattleTransition(u8 taskId) {
   MENU_TASK_SET_MENU(sBattleTransitionActions);
}