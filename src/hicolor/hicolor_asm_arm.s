
.set FACILITATE_RUNNING_FROM_IWRAM, 1

@ r0     == argument: scanline mapping pointer for this scanline
@ r1     == argument: HiColor palette  pointer base
@ r2     == destination pointer (palette memory)
@ r3     == scratch variable (HiColor palette index read from r0 / HiColor palette to copy from)
@ r4-r11 == scratch variable (for 32-byte copy)
@ r12    == scratch variable (dword from r0; only if copy-by-dword enabled)

.set OBJ_PLTT,           0x05000200
.set PLTT_SIZE_4BPP,     0x20
.set COLORS_PER_PALETTE, 16

@ 16 colors * 2 bytes per color = 32 bytes == 1 << 5
.set BITSHIFT_PER_PALETTE, 5

.set COPY_BY_DWORD, 1

.macro copypalette
   lsl   r3, r3, #BITSHIFT_PER_PALETTE
   add   r3, r1
   ldmia r3!,{r4-r11}
   stmia r2!,{r4-r11}
.endm

.macro loop_body index:req
.set IS_NOT_LAST_ITERATION, (\index - COLORS_PER_PALETTE)
   ldrb  r3, [r0, #\index]
   cmp   r3, #255
   beq   HiColor_HBlank_Asm_lAfterCopy\index
   copypalette
   .if IS_NOT_LAST_ITERATION
      sub   r2, #PLTT_SIZE_4BPP
   .endif
HiColor_HBlank_Asm_lAfterCopy\index:
   .if IS_NOT_LAST_ITERATION
      add   r2, #PLTT_SIZE_4BPP
   .endif
.endm

.macro loop_body_dword_fragment index:req
   add   r2,  #PLTT_SIZE_4BPP
   lsr   r12, r12, #8
   and   r3,  r12, #0xFF
   cmp   r3,  #255
   beq   HiColor_HBlank_Asm_lAfterCopyChunk\index
   copypalette
   .if IS_NOT_LAST_BLOCK
      sub   r2,  #PLTT_SIZE_4BPP
   .endif
HiColor_HBlank_Asm_lAfterCopyChunk\index:
.endm
@
.macro loop_body_dword index:req
.set IS_NOT_LAST_BLOCK, (\index - 12)
   ldr   r12, [r0, #\index]
   and   r3,  r12, #0xFF
   cmp   r3, #255
   beq   HiColor_HBlank_Asm_lAfterCopyChunkA\index
   copypalette
   sub   r2, #PLTT_SIZE_4BPP
HiColor_HBlank_Asm_lAfterCopyChunkA\index:
   loop_body_dword_fragment B\index
   loop_body_dword_fragment C\index
   loop_body_dword_fragment D\index
   .if IS_NOT_LAST_BLOCK
      add   r2,  #PLTT_SIZE_4BPP
   .endif
.endm

.align 4, 0
.global HiColor_HBlank_AsmARM
.arch armv4t
.arm
.type HiColor_HBlank_AsmARM, %function
HiColor_HBlank_AsmARM:
   .if COPY_BY_DWORD
      stmfd sp!,{r4-r12,lr}
   .else
      stmfd sp!,{r4-r11,lr}
   .endif
   ldr  r2, =OBJ_PLTT
   .if COPY_BY_DWORD
      loop_body_dword 0
      loop_body_dword 4
      loop_body_dword 8
      loop_body_dword 12
   .else
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
   .endif
HiColor_HBlank_AsmARM_Epilogue:
   .if COPY_BY_DWORD
      ldmia sp!,{r4-r12,lr}
   .else
      ldmia sp!,{r4-r11,lr}
   .endif
   bx lr
.size HiColor_HBlank_AsmARM, .-HiColor_HBlank_AsmARM

.if FACILITATE_RUNNING_FROM_IWRAM
@
@ THUMB can't jump directly from code that lives in ROM to code 
@ that lives in IWRAM. There's no equivalent to x86's `CALL EAX` 
@ or `JMP EAX`. However, ARM has such a capability, so we have a 
@ small ARM shim that takes a pointer to the target subroutine 
@ as an additional argument, while forwarding the original args.
@
.align 4, 0
.global HiColor_HBlank_AsmARM_ViaArm
.arch armv4t
.arm
.type HiColor_HBlank_AsmARM_ViaArm, %function
HiColor_HBlank_AsmARM_ViaArm:
   bx r2
.size HiColor_HBlank_AsmARM_ViaArm, .-HiColor_HBlank_AsmARM_ViaArm
.endif
