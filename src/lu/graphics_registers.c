#include "lu/graphics_registers.h"
#include "gpu_regs.h"

extern void ResetBlendRegisters(void) {
   SetGpuReg(REG_OFFSET_BLDCNT, 0);
   SetGpuReg(REG_OFFSET_BLDALPHA, 0);
   SetGpuReg(REG_OFFSET_BLDY, 0);
}
extern void ResetScreenWindows(void) {
   SetGpuReg(REG_OFFSET_WIN0H,  0);
   SetGpuReg(REG_OFFSET_WIN0V,  0);
   SetGpuReg(REG_OFFSET_WIN1H,  0);
   SetGpuReg(REG_OFFSET_WIN1V,  0);
   SetGpuReg(REG_OFFSET_WININ,  0);
   SetGpuReg(REG_OFFSET_WINOUT, 0);
}

extern void GetBlendRegisters(struct ColorEffectParams* dst) {
   dst->bldcnt   = GetGpuReg(REG_OFFSET_BLDCNT);
   dst->bldalpha = GetGpuReg(REG_OFFSET_BLDALPHA);
   dst->bldy     = GetGpuReg(REG_OFFSET_BLDY);
}
extern void SetBlendRegisters(const struct ColorEffectParams* src) {
   SetGpuReg(REG_OFFSET_BLDCNT,   src->bldcnt);
   SetGpuReg(REG_OFFSET_BLDALPHA, src->bldalpha);
   SetGpuReg(REG_OFFSET_BLDY,     src->bldy);
}

extern void SetScreenWindowParams(const struct ScreenWindowParams* src) {
   SetGpuReg(REG_OFFSET_WIN0H,  src->bounds.win_0.h.io_register);
   SetGpuReg(REG_OFFSET_WIN0V,  src->bounds.win_0.v.io_register);
   SetGpuReg(REG_OFFSET_WIN1H,  src->bounds.win_1.h.io_register);
   SetGpuReg(REG_OFFSET_WIN1V,  src->bounds.win_1.v.io_register);
   SetGpuReg(REG_OFFSET_WININ,  src->mask.win_in);
   SetGpuReg(REG_OFFSET_WINOUT, src->mask.win_out);
}

extern void SetScreenWindowsEnabled(u8 windows, bool8 enabled) {
   const u8 all_windows = (SCREEN_WINDOW_0 | SCREEN_WINDOW_1 | SCREEN_WINDOW_OBJ);
   
   u16 to_set   =  (windows & all_windows) << 13;
   u16 to_clear = (~windows & all_windows) << 13;
   if (to_set) {
      SetGpuRegBits(REG_OFFSET_DISPCNT, to_set);
   }
   if (to_clear) {
      ClearGpuRegBits(REG_OFFSET_DISPCNT, to_clear);
   }
}