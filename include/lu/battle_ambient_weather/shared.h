#ifndef GUARD_LU_BATTLE_AMBIENT_WEATHER_SHARED_H
#define GUARD_LU_BATTLE_AMBIENT_WEATHER_SHARED_H

#include "gba/types.h"

extern u8 gAmbientWeatherTaskId;

// task data fields
#define tWeatherType     data[0]
// ...
#define tTargetPalettesA data[11]
#define tTargetPalettesB data[12]
#define tDelay           data[13] // high bits: frames between steps; low bits: current counter
#define tBlendRange      data[14] // high bits: from; low bits: to
#define tBlendColor      data[15] // e.g. RGB_BLACK

extern u16 GetActiveAmbientWeather(void);

extern void TaskHelper_QueueBlendColors(u8 taskId, u32 palettes, u8 delay, u8 blendFrom, u8 blendTo, u16 blendColor);
extern bool8 TaskHelper_BlendColors(u8 taskId); // returns TRUE when blend done

#endif