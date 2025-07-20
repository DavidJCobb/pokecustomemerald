#ifndef GUARD_LU_VUI_VIRTUAL_H
#define GUARD_LU_VUI_VIRTUAL_H

struct VUI_VirtualFunctionTable {
   struct VUI_VirtualFunctionTable* superclass_vtable;
};

// No-op function called by `v_invoke` when a virtual function isn't defined.
extern void _VUI_Invoke_Stub(...);

#define v_can_invoke(this, func) (!!(this) && !!((this)->functions->func))
#define v_invoke(this, func) ( ((this)->functions->func)((this) v_invoke_impl_close

// Implementation detail for above macro.
#define v_invoke_impl_close(...) ,##__VA_ARGS__))

#endif