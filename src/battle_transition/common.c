#include "battle_transition/common.h"
#include <string.h> // memset
#include "gba/gba.h"
#include "constants/rgb.h"
#include "field_camera.h" // GetCameraOffsetWithPan
#include "palette.h"
#include "sprite.h"
#include "trig.h"

EWRAM_DATA struct BattleTransitionData* gBattleTransitionData = NULL;

extern void BattleTransitionCommon_InitTransitionData(void) {
   memset(gBattleTransitionData, 0, sizeof(*gBattleTransitionData));
   GetCameraOffsetWithPan(&gBattleTransitionData->cameraX, &gBattleTransitionData->cameraY);
}

extern void BattleTransitionCommon_GetBg0TilesDst(u16** tilemap, u16** tileset) {
   u16 screenBase = REG_BG0CNT >> 8;
   u16 charBase   = REG_BG0CNT >> 2;

   screenBase <<= 11;
   charBase   <<= 14;

   *tilemap = (u16*)(BG_VRAM + screenBase);
   *tileset = (u16*)(BG_VRAM + charBase);
}
extern void BattleTransitionCommon_GetBg0TilemapDst(u16** tilemap) {
   u16 charBase = REG_BG0CNT >> 2;
   charBase <<= 14;
   *tilemap = (u16*)(BG_VRAM + charBase);
}

extern void BattleTransitionCommon_FadeScreenBlack(void) {
   BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
}

extern void BattleTransitionCommon_SetCircularMask(
   u16* buffer,
   s16  centerX,
   s16  centerY,
   s16  radius
) {
   memset(buffer, 10, DISPLAY_HEIGHT * sizeof(u16));
   for (s16 i = 0; i < 64; i++) {
      s16 sinResult = Sin(i, radius);
      s16 cosResult = Cos(i, radius);

      s16 drawXLeft = centerX - sinResult;
      s16 drawX     = centerX + sinResult;
      s16 drawYTop  = centerY - cosResult;
      s16 drawYBott = centerY + cosResult;

      if (drawXLeft < 0)
         drawXLeft = 0;
      if (drawX > DISPLAY_WIDTH)
         drawX = DISPLAY_WIDTH;
      if (drawYTop < 0)
         drawYTop = 0;
      if (drawYBott > DISPLAY_HEIGHT - 1)
         drawYBott = DISPLAY_HEIGHT - 1;

      drawX |= (drawXLeft << 8);
      buffer[drawYTop]  = drawX;
      buffer[drawYBott] = drawX;

      cosResult = Cos(i + 1, radius);
      s16 drawYTopNext  = centerY - cosResult;
      s16 drawYBottNext = centerY + cosResult;

      if (drawYTopNext < 0)
         drawYTopNext = 0;
      if (drawYBottNext > DISPLAY_HEIGHT - 1)
         drawYBottNext = DISPLAY_HEIGHT - 1;

      while (drawYTop > drawYTopNext)
         buffer[--drawYTop] = drawX;
      while (drawYTop < drawYTopNext)
         buffer[++drawYTop] = drawX;

      while (drawYBott > drawYBottNext)
         buffer[--drawYBott] = drawX;
      while (drawYBott < drawYBottNext)
         buffer[++drawYBott] = drawX;
    }
}
extern void BattleTransitionCommon_SetSinWave(
   s16* array,
   s16  sinAdd,
   s16  index,
   s16  indexIncrementer,
   s16  amplitude,
   s16  arrSize
) {
   for (u8 i = 0; arrSize > 0; arrSize--, i++, index += indexIncrementer)
      array[i] = sinAdd + Sin(index & 0xFF, amplitude);
}

extern void BattleTransitionCommon_VBlankCB(void) {
   LoadOam();
   ProcessSpriteCopyRequests();
   TransferPlttBuffer();
}