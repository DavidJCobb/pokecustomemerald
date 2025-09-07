#include "global.h"
#include "time_events.h"
#include "event_data.h"
#include "field_weather.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "overworld.h"
#include "rtc.h"
#include "script.h"
#include "task.h"

#include "custom_game_options/options.h"

static u32 GetMirageRnd(void)
{
    u32 hi = VarGet(VAR_MIRAGE_RND_H);
    u32 lo = VarGet(VAR_MIRAGE_RND_L);
    return (hi << 16) | lo;
}

static void SetMirageRnd(u32 rnd)
{
    VarSet(VAR_MIRAGE_RND_H, rnd >> 16);
    VarSet(VAR_MIRAGE_RND_L, rnd);
}

// unused
void InitMirageRnd(void)
{
    SetMirageRnd((Random() << 16) | Random());
}

void UpdateMirageRnd(u16 days)
{
    s32 rnd = GetMirageRnd();
    while (days)
    {
        rnd = ISO_RANDOMIZE2(rnd);
        days--;
    }
    SetMirageRnd(rnd);
}

bool8 IsMirageIslandPresent(void) {
   if (gCustomGameOptions.events.mirage_island.rarity == 0) {
      return TRUE;
   }
   if (gCustomGameOptions.events.mirage_island.rarity == 17) {
      return FALSE;
   }
   
   u16 rnd  = GetMirageRnd() >> 16;
   u16 mask = 0xFFFF;
   {
      u8 bits = gCustomGameOptions.events.mirage_island.rarity;
      if (bits < 16) {
         mask = (1 << bits) - 1;
         rnd  &= mask;
      }
   }

   for (int i = 0; i < PARTY_SIZE; i++) {
      if (!GetMonData(&gPlayerParty[i], MON_DATA_SPECIES))
         continue;
      
      u16 personality = GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY) & mask;
      
      if (personality == rnd)
         return TRUE;
   }
   
   if (gCustomGameOptions.events.mirage_island.include_pc) {
      for(u8 i = 0; i < TOTAL_BOXES_COUNT; ++i) {
         for(u8 j = 0; j < IN_BOX_COUNT; ++j) {
            const struct BoxPokemon* mon = &gPokemonStoragePtr->boxes[i][j];
            if (!mon->hasSpecies)
               continue;
            u16 personality = mon->personality & mask;
            if (personality == rnd)
               return TRUE;
         }
      }
   }

   return FALSE;
}

void UpdateShoalTideFlag(void)
{
    static const u8 tide[] =
    {
        1, // 00
        1, // 01
        1, // 02
        0, // 03
        0, // 04
        0, // 05
        0, // 06
        0, // 07
        0, // 08
        1, // 09
        1, // 10
        1, // 11
        1, // 12
        1, // 13
        1, // 14
        0, // 15
        0, // 16
        0, // 17
        0, // 18
        0, // 19
        0, // 20
        1, // 21
        1, // 22
        1, // 23
    };

    if (IsMapTypeOutdoors(GetLastUsedWarpMapType()))
    {
        RtcCalcLocalTime();
        if (tide[gLocalTime.hours])
            FlagSet(FLAG_SYS_SHOAL_TIDE);
        else
            FlagClear(FLAG_SYS_SHOAL_TIDE);
    }
}

static void Task_WaitWeather(u8 taskId)
{
    if (IsWeatherChangeComplete())
    {
        ScriptContext_Enable();
        DestroyTask(taskId);
    }
}

void WaitWeather(void)
{
    CreateTask(Task_WaitWeather, 80);
}

void InitBirchState(void)
{
    *GetVarPointer(VAR_BIRCH_STATE) = 0;
}

void UpdateBirchState(u16 days)
{
    u16 *state = GetVarPointer(VAR_BIRCH_STATE);
    *state += days;
    *state %= 7;
}
