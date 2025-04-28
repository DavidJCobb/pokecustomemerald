#include "lu/battle_ambient_weather/core.h"
#include "lu/battle_ambient_weather/shared.h"
#include "lu/battle_ambient_weather/hail.h"
#include "lu/battle_ambient_weather/rain.h"
#include "lu/battle_ambient_weather/sandstorm.h"
#include "lu/battle_ambient_weather/sunny.h"
#include "global.h" // *sigh*
#include "constants/battle.h"
#include "battle.h"
#include "task.h"
#include "gba/isagbprint.h"

extern void InitBattleAmbientWeather(void) {
   gAmbientWeatherTaskId = TASK_NONE;
}
extern void StartBattleAmbientWeatherAnim(void) {
   DebugPrintf("[Battle Ambient Weather] Asked to start. Weather is %04X. Pre-existing task, if any, is ID %u.", gBattleWeather, gAmbientWeatherTaskId);
   if (gAmbientWeatherTaskId != TASK_NONE) {
      return;
   }
   if (gBattleWeather & B_WEATHER_RAIN) {
      StartBattleAmbientWeatherAnim_Rain();
   } else if (gBattleWeather & B_WEATHER_SANDSTORM) {
      StartBattleAmbientWeatherAnim_Sandstorm();
   } else if (gBattleWeather & B_WEATHER_HAIL) {
      StartBattleAmbientWeatherAnim_Hail();
   } else if (gBattleWeather & B_WEATHER_SUN) {
      StartBattleAmbientWeatherAnim_Sunny();
   }
   if (gAmbientWeatherTaskId != TASK_NONE) {
      gTasks[gAmbientWeatherTaskId].tWeatherType = gBattleWeather;
   }
}
extern void StopBattleAmbientWeatherAnim(bool8 instant) {
   u16 weather = GetActiveAmbientWeather();
   DebugPrintf("[Battle Ambient Weather] Asked to stop (instantly? %u). Existing weather is %04X.", instant, weather);
   if (weather & B_WEATHER_RAIN) {
      StopBattleAmbientWeatherAnim_Rain(instant);
   } else if (weather & B_WEATHER_SANDSTORM) {
      StopBattleAmbientWeatherAnim_Sandstorm(instant);
   } else if (weather & B_WEATHER_HAIL) {
      StopBattleAmbientWeatherAnim_Hail(instant);
   } else if (weather & B_WEATHER_SUN) {
      StopBattleAmbientWeatherAnim_Sunny(instant);
   }
}