@.include "./../../asm/macros/function.inc"

.set OBJ_PLTT,       0x05000200
.set PLTT_SIZE_4BPP, 0x20

@ 16 colors * 2 bytes per color = 32 bytes == 1 << 5
.set BITSHIFT_PER_PALETTE, 5

@ Syscall. Copies memory in 32-byte blocks.
@    r0: source
@    r1: destination
@    r2: size in dwords
@ Increments r1 after each 32-byte copy.
@ Registers  r2 through r9 will be clobbered.
.macro cpufastset
   swi #0x0C
.endm
@
@ As an aside, CpuSet increments r3 and clobbers r0 and r5.

@
@ Registers used:
@
@    r0:  Current scanline VRAM-to-HiColor index.
@    r1:  Destination pointer (VRAM palette memory).
@    r2:  Constant argument to CpuFastSet syscall.
@    r4:  Scanline base pointer. Transferred in from arg1 (r0).
@    r10: Blending palettes pointer. Transferred in from arg2 (r1).
@
@ Only registers above r9 can avoid being clobbered by CpuFastSet, 
@ but registers above r7 can only be used in specific opcodes. If 
@ we use these registers whenever possible, we reduce the amount 
@ of stuff we have to push and pop around each CpuFastSet call.
@

.macro loop_body index_a:req
   ldrb r0, [r4, #\index_a]
   cmp  r0, #255
   beq  HiColor_HBlank_Asm_lAfterCopy\index_a
   lsl  r0, r0, #BITSHIFT_PER_PALETTE
   add  r0, r10
   push {r1,r2,r4}
   cpufastset
   pop  {r1,r2,r4}
HiColor_HBlank_Asm_lAfterCopy\index_a :
   add  r1, #PLTT_SIZE_4BPP
.endm

.align 2, 0
.global HiColor_HBlank_Asm
.thumb
.thumb_func
.type HiColor_HBlank_Asm, %function
HiColor_HBlank_Asm:
   push {r2,r4,lr}
   mov  r4, r10
   push {r4}
   @
   mov  r4,  r0
   mov  r10, r1
   ldr  r1,  =OBJ_PLTT
   mov  r2,  #(PLTT_SIZE_4BPP / 4)
   loop_body 0
   loop_body 1
   loop_body 2
   loop_body 3
   loop_body 4
   loop_body 5
   loop_body 6
   loop_body 7
   loop_body 8
   loop_body 9
   loop_body 10
   loop_body 11
   loop_body 12
   loop_body 13
   loop_body 14
   loop_body 15
   @
   pop  {r4}
   mov  r10, r4
   pop  {r2,r4,pc}
.size HiColor_HBlank_Asm, .-HiColor_HBlank_Asm
