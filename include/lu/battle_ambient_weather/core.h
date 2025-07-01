#ifndef GUARD_LU_BATTLE_AMBIENT_WEATHER_H
#define GUARD_LU_BATTLE_AMBIENT_WEATHER_H

#include "gba/types.h"

extern void InitBattleAmbientWeather(void);
extern void StartBattleAmbientWeatherAnim(void);
extern void StopBattleAmbientWeatherAnim(bool8 instant);
extern bool8 IsBattleAmbientWeatherPlaying(void);

#endif