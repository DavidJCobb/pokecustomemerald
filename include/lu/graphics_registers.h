#ifndef GUARD_LU_GRAPHICS_REGISTERS_H
#define GUARD_LU_GRAPHICS_REGISTERS_H

#include "gba/types.h"
#include "gba/io_reg.h"

struct ColorEffectParams {
   union {
      struct {
         u8 target1Layers : 6;
         u8 effect        : 2;
         u8 target2Layers : 6;
         u8 unused        : 2;
      };
      u16 bldcnt;
   };
   union {
      struct {
         u8 target1Coefficient : 5; // [0, 16]
         u8 unused_a           : 3;
         u8 target2Coefficient : 5; // [0, 16]
         u8 unused_b           : 3;
      } alpha;
      u16 bldalpha;
   };
   union {
      struct {
         u8 coefficient : 5;
      } brightness;
      u16 bldy;
   };
};

enum {
   COLOR_EFFECT_NONE = 0,
   COLOR_EFFECT_ALPHA_BLEND = 1,
   COLOR_EFFECT_BRIGHTNESS_INCREASE = 2,
   COLOR_EFFECT_BRIGHTNESS_DECREASE = 3,
};

enum {
   COLOR_EFFECT_LAYER_NONE = 0,
   COLOR_EFFECT_LAYER_BG0  = 1,
   COLOR_EFFECT_LAYER_BG1  = 2,
   COLOR_EFFECT_LAYER_BG2  = 4,
   COLOR_EFFECT_LAYER_BG3  = 8,
   COLOR_EFFECT_LAYER_OBJ  = 16,
   COLOR_EFFECT_LAYER_BACKDROP = 32,
   
   COLOR_EFFECT_LAYER_ALL = 63,
};

struct ScreenWindowExtent {
   union {
      struct {
         u8 start;
         u8 end;
      };
      u16 io_register;
   };
};

struct ScreenWindowMaskParams {
   u8    layers : 5;
   bool8 enable_color_effect : 1;
};

struct ScreenWindowParams {
   struct {
      struct {
         struct ScreenWindowExtent h; // WIN0H
         struct ScreenWindowExtent v; // WIN0V
      } win_0;
      struct {
         struct ScreenWindowExtent h; // WIN1H
         struct ScreenWindowExtent v; // WIN1V
      } win_1;
   } bounds;
   struct {
      union {
         struct __attribute__ ((__packed__)) {
            struct ScreenWindowMaskParams win_0;
            u8     unused_a : 2;
            struct ScreenWindowMaskParams win_1;
            u8     unused_b : 2;
         };
         u16 win_in; // WININ
      };
      union {
         struct __attribute__ ((__packed__)) {
            struct ScreenWindowMaskParams win_outside;
            u8     unused_c : 2;
            struct ScreenWindowMaskParams win_obj;
            u8     unused_d : 2;
         };
         u16 win_out; // WINOUT
      };
   } mask;
};

enum {
   SCREEN_WINDOW_MASK_LAYER_NONE = 0,
   
   SCREEN_WINDOW_MASK_LAYER_BG0 = 1,
   SCREEN_WINDOW_MASK_LAYER_BG1 = 2,
   SCREEN_WINDOW_MASK_LAYER_BG2 = 4,
   SCREEN_WINDOW_MASK_LAYER_BG3 = 8,
   SCREEN_WINDOW_MASK_LAYER_OBJ = 16,
   
   SCREEN_WINDOW_MASK_LAYER_BG_ALL =
      SCREEN_WINDOW_MASK_LAYER_BG0 | SCREEN_WINDOW_MASK_LAYER_BG1 | SCREEN_WINDOW_MASK_LAYER_BG2 | SCREEN_WINDOW_MASK_LAYER_BG3,
};

extern void ResetBlendRegisters(void);
extern void ResetScreenWindows(void);

extern void GetBlendRegisters(struct ColorEffectParams*);
extern void SetBlendRegisters(const struct ColorEffectParams*);
extern void SetScreenWindowParams(const struct ScreenWindowParams*);

enum {
   SCREEN_WINDOW_0   = 1,
   SCREEN_WINDOW_1   = 2,
   SCREEN_WINDOW_OBJ = 4,
};

extern void SetScreenWindowsEnabled(u8 windows, bool8 enabled);

#endif