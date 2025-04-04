#ifndef GUARD_LU_UI_WIDGET_SCROLLBAR_V
#define GUARD_LU_UI_WIDGET_SCROLLBAR_V

#include "lu/widgets/scrollbar_common.h"

// Allocates initial memory and initializes some VRAM-related state.
extern void InitScrollbarV(struct LuScrollbar*, const struct LuScrollbarGraphicsParams*);

extern void RepaintScrollbarV(struct LuScrollbar*);

// Frees heap-allocated memory.
extern void DestroyScrollbarV(struct LuScrollbar*);

#endif

