@.include "./../../asm/macros/function.inc"

.set OBJ_PLTT,       0x05000200
.set PLTT_SIZE_4BPP, 0x20

@ 16 colors * 2 bytes per color = 32 bytes == 1 << 5
.set BITSHIFT_PER_PALETTE, 5

@ r0: source
@ r1: destination
@ r2: size in dwords
.macro cpufastset
   swi #0x0C
.endm

@ r4: argument: scanline base     pointer
@ r5: argument: palettes blending pointer

.macro loop_body index_a:req
   ldrb r0, [r4, #\index_a]
   cmp  r0, #255
   beq  HiColor_HBlank_Asm_lAfterCopy\index_a
   lsl  r0, r0, #BITSHIFT_PER_PALETTE
   add  r0, r5
   push {r1,r2,r4,r5}
   push {lr}
   cpufastset
   pop  {r1}
   mov  lr, r1
   pop  {r1,r2,r4,r5}
HiColor_HBlank_Asm_lAfterCopy\index_a :
   add  r1, #PLTT_SIZE_4BPP
.endm

@thumb_func_start HiColor_HBlank_Asm
.align 2, 0
.global HiColor_HBlank_Asm
.thumb
.thumb_func
.type HiColor_HBlank_Asm, %function
HiColor_HBlank_Asm:
   push {r2,r4,r5,lr}
   mov  r4, r0
   mov  r5, r1
   ldr  r1, =OBJ_PLTT
   mov  r2, #(PLTT_SIZE_4BPP / 4)
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
   pop {r2,r4,r5,pc}
@thumb_func_end HiColor_HBlank_Asm
.size HiColor_HBlank_Asm, .-HiColor_HBlank_Asm
