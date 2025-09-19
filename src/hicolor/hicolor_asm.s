
.set OBJ_PLTT,           0x05000200
.set PLTT_SIZE_4BPP,     0x20
.set COLORS_PER_PALETTE, 16

.set FACILITATE_RUNNING_FROM_IWRAM, 0

@ 16 colors * 2 bytes per color = 32 bytes == 1 << 5
.set BITSHIFT_PER_PALETTE, 5

@
@ The cycle budget for h-blank is 272. We have a little less 
@ than that, because we get executed indirectly through gMain 
@ machinery, which will burn a few cycles.
@

@ Syscall. Copies memory in 32-byte blocks.
@    r0: source
@    r1: destination
@    r2: size in dwords
@ Increments r1 after each 32-byte copy.
@ Registers  r2 through r9 will be clobbered.
.macro cpufastset
   @
   @ Under the hood, this is implemented as LDMIA Rb!,{r2-r9} 
   @ and STMIA r1!,{r2-r9}, so I would expect the respective 
   @ cycle counts to be 7S+2N and 7S+1N+1I, for a total of 
   @ 14S+3N+1I. The SWI opcode has an overhead of 2S+1N, so 
   @ the final figure would be 16S+4N+1I.
   @
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
@    r11: Scratch register; MOV is cheaper than PUSH/POP.
@
@ Only registers above r9 can avoid being clobbered by CpuFastSet, 
@ but registers above r7 can only be used in specific opcodes. If 
@ we use these registers whenever possible, we reduce the amount 
@ of stuff we have to push and pop around each CpuFastSet call.
@

.macro loop_body index_a:req
.set IS_NOT_LAST_ITERATION, (\index_a - COLORS_PER_PALETTE - 1)
   @
   ldrb r0, [r4, #\index_a]
   cmp  r0, #255
   beq  HiColor_HBlank_Asm_lAfterCopy\index_a
   lsl  r0, r0, #BITSHIFT_PER_PALETTE
   add  r0, r10
   @
   @ Where r0 was previously a HiColor palette index P, it is now 
   @ a pointer to sHiColorState.palettes.blending[P].
   @
   @ r2 will be clobbered by CpuFastSet, but we only use it for 
   @ CpuFastSet, so we should just set it immediately prior to 
   @ each CpuFastSet call.
   @
   mov  r2, #(PLTT_SIZE_4BPP / 4)
   cpufastset
   @
   @ We have to protect registers r1 and r4 from being altered or 
   @ clobbered by CpuFastSet. The intuitively obvious way to do 
   @ this is via PUSH and POP instructions, but those instructions 
   @ incur non-sequential (N) cycles, which are slower than just 
   @ sequential (S) cycles. So instead...
   @
   @  - r1 is just incremented, so we can subtract it (1S) after. 
   @    (CpuFastSet only runs conditionally but increments it by 
   @    32, and we want to unconditionally increment it by 32. We 
   @    could just jump past the non-CpuFastSet addition, except 
   @    that a jump is 2S+1N, whereas the redundant SUB/ADD totals 
   @    up to 2S, which is cheaper.)
   @ 
   @  - r4 can be backed up in a high register (r11), and restored 
   @    via a move. If we back it up at the start of our subroutine, 
   @    then this incurs just the 1S from restoring it, per loop.
   @
   @ This is ultimately preferable to PUSHing and POPping both of 
   @ the registers ((1S+2N+1I) * 2). The totals are 2S per loop 
   @ for this version, versus 2S+4N+2I if we used the stack for r1 
   @ and r4.
   @
   .if IS_NOT_LAST_ITERATION
      mov  r4, r11
      sub  r1, #PLTT_SIZE_4BPP
   .endif
HiColor_HBlank_Asm_lAfterCopy\index_a :
   .if IS_NOT_LAST_ITERATION
      add  r1, #PLTT_SIZE_4BPP
   .endif
.endm

.align 2, 0
.global HiColor_HBlank_Asm
.thumb
.thumb_func
.type HiColor_HBlank_Asm, %function
HiColor_HBlank_Asm:
   @
   @ Prologue. ARM EABI considers r0-r3 as scratch registers. Only 
   @ low registers (r0-r7) and call-related registers (lr/pc) can 
   @ be pushed/popped directly; high registers need to be copied  
   @ into low registers and then pushed (and the same in reverse 
   @ to pop).
   @
   @ Registers r2-r9 are clobbered by CpuFastSet, and we use some 
   @ high registers to work around that (see above), so we pretty 
   @ much have to protect all non-scratch, non-special registers.
   @
   push {r4-r7,lr}
   mov  r2,  r8
   mov  r3,  r9
   mov  r4,  r10
   mov  r5,  r11
   push {r2,r3,r4,r5}
   @
   @ Body.
   @
   mov  r4,  r0
   mov  r10, r1
   ldr  r1,  =OBJ_PLTT
   mov  r11, r4
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
   @ Epilogue.
   @
   pop  {r2,r3,r4,r5}
   mov  r8,  r2
   mov  r9,  r3
   mov  r10, r4
   mov  r11, r5
   pop  {r4-r7,pc}
.size HiColor_HBlank_Asm, .-HiColor_HBlank_Asm

.if FACILITATE_RUNNING_FROM_IWRAM
@
@ THUMB can't jump directly from code that lives in ROM to code 
@ that lives in IWRAM. There's no equivalent to x86's `CALL EAX` 
@ or `JMP EAX`. However, ARM has such a capability, so we have a 
@ small ARM shim that takes a pointer to the target subroutine 
@ as an additional argument, while forwarding the original args.
@
.align 4, 0
.global HiColor_HBlank_Asm_ViaArm
.arch armv4t
.arm
.type HiColor_HBlank_Asm_ViaArm, %function
HiColor_HBlank_Asm_ViaArm:
   orr r2, r2, #1 @ Set THUMB flag.
   bx  r2
.size HiColor_HBlank_Asm_ViaArm, .-HiColor_HBlank_Asm_ViaArm
.endif
