#include "battle_transition/common.h"
#include "battle_transition/common_black_wipe.h"
#include "gba/defines.h" // TRUE, FALSE

// Below are data defines for InitBlackWipe and UpdateBlackWipe, for the TransitionData data array.
// These will be re-used by any transitions that use these functions.
#define tWipeStartX data[0]
#define tWipeStartY data[1]
#define tWipeCurrX  data[2]
#define tWipeCurrY  data[3]
#define tWipeEndX   data[4]
#define tWipeEndY   data[5]
#define tWipeXMove  data[6]
#define tWipeYMove  data[7]
#define tWipeXDist  data[8]
#define tWipeYDist  data[9]
#define tWipeTemp   data[10]

extern void BattleTransitionCommon_InitBlackWipe(
   s16* data,
   s16  startX,
   s16  startY,
   s16  endX,
   s16  endY,
   s16  xMove,
   s16  yMove
) {
   tWipeStartX = startX;
   tWipeStartY = startY;
   tWipeCurrX  = startX;
   tWipeCurrY  = startY;
   tWipeEndX   = endX;
   tWipeEndY   = endY;
   tWipeXMove  = xMove;
   tWipeYMove  = yMove;
   tWipeXDist  = endX - startX;
   if (tWipeXDist < 0) {
      // If end was less than start, reverse direction
      tWipeXDist = -tWipeXDist;
      tWipeXMove = -xMove;
   }
   tWipeYDist = endY - startY;
   if (tWipeYDist < 0) {
      // If end was less than start, reverse direction
      tWipeYDist = -tWipeYDist;
      tWipeYMove = -yMove;
   }
   tWipeTemp = 0;
}

extern bool8 BattleTransitionCommon_UpdateBlackWipe(
   s16*  data,
   bool8 xExact,
   bool8 yExact
) {
   if (tWipeXDist > tWipeYDist) {
      // X has further to move, move it first
      tWipeCurrX += tWipeXMove;

      // If it has been far enough since Y's
      // last move then move it too
      tWipeTemp += tWipeYDist;
      if (tWipeTemp > tWipeXDist)  {
         tWipeCurrY += tWipeYMove;
         tWipeTemp  -= tWipeXDist;
      }
   } else {
      // Y has further to move, move it first
      tWipeCurrY += tWipeYMove;

      // If it has been far enough since X's
      // last move then move it too
      tWipeTemp += tWipeXDist;
      if (tWipeTemp > tWipeYDist) {
         tWipeCurrX += tWipeXMove;
         tWipeTemp  -= tWipeYDist;
      }
   }

   u8 numFinished = 0;

   // Has X coord reached end?
   if (
      (tWipeXMove > 0 && tWipeCurrX >= tWipeEndX)
   || (tWipeXMove < 0 && tWipeCurrX <= tWipeEndX)
   ) {
      numFinished++;
      if (xExact)
         tWipeCurrX = tWipeEndX;
   }

   // Has Y coord reached end?
   if (
      (tWipeYMove > 0 && tWipeCurrY >= tWipeEndY)
   || (tWipeYMove < 0 && tWipeCurrY <= tWipeEndY)
   ) {
      numFinished++;
      if (yExact)
         tWipeCurrY = tWipeEndY;
   }

   return (numFinished == 2) ? TRUE : FALSE;
}