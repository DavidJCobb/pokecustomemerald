	.include "constants/gba_constants.inc"

	.syntax unified

	.arm

	.align 2, 0
Init::
	mov r0, #PSR_IRQ_MODE
	msr cpsr_cf, r0
	ldr sp, sp_irq
	mov r0, #PSR_SYS_MODE
	msr cpsr_cf, r0
	ldr sp, sp_sys
	ldr r1, =INTR_VECTOR
	adr r0, IntrMain
	str r0, [r1]
	.if MODERN
	mov r0, #255 @ RESET_ALL
	svc #1 << 16
	.endif @ MODERN
	ldr r1, =AgbMain + 1
	mov lr, pc
	bx r1
	b Init

	.align 2, 0
sp_sys: .word IWRAM_END - 0x1c0
sp_irq: .word IWRAM_END - 0x60

	.pool

	.arm
	.align 2, 0
IntrMain::
	mov r3, #REG_BASE
	add r3, r3, #OFFSET_REG_IE
	ldr r2, [r3]
	@
	@ We have stored both IE (Interrupt Enable) and IF (IRQ Request Flags) 
	@ in r2, with IE in the low 16 bits and IF in the high 16 bits.
	@
	@ We are about to set r1 to the IME register. The way we access it is 
	@ a bit strange, to take advantage of the pointer we already have in 
	@ r3.
	@
	ldrh r1, [r3, #OFFSET_REG_IME - OFFSET_REG_IE]
	mrs r0, spsr
	stmfd sp!, {r0-r3,lr}
	@
	@ We just preserved SPSR, IME, IE, and IF on the stack. We will restore 
	@ them to their original values at the end of this subroutine.
	@
	mov r0, #0
	strh r0, [r3, #OFFSET_REG_IME - OFFSET_REG_IE]
	and r1, r2, r2, lsr #16
	mov r12, #0
	@
	@ r1 has been set to IE & IF. We can use `ANDS r0, r1, #flag` to check 
	@ whether a given interrupt is both enabled and active.
	@
	@ We have a table of interrupt handlers. r12 is an index (in bytes) into 
	@ that table, used to invoke a handler if its interrupt is active.
	@
	@ We also just set IME to 0 to blanket disable all interrupts. We will 
	@ turn it on after we (potentially) handle the VCOUNT interrupt.
	@
	ands r0, r1, #INTR_FLAG_VCOUNT
	bne IntrMain_FoundIntr
	@
	@ Move on to the next interrupt, including by setting IME to 1 to force 
	@ all interrupts to not-globally-disabled.
	@
	add r12, r12, 0x4
	mov r0, 0x1
	strh r0, [r3, #OFFSET_REG_IME - OFFSET_REG_IE]
	@
	@ Now we can check the rest of the interrupts one after another.
	@
	ands r0, r1, #INTR_FLAG_SERIAL
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	ands r0, r1, #INTR_FLAG_TIMER3
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	ands r0, r1, #INTR_FLAG_HBLANK
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	ands r0, r1, #INTR_FLAG_VBLANK
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	ands r0, r1, #INTR_FLAG_TIMER0
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	ands r0, r1, #INTR_FLAG_TIMER1
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	ands r0, r1, #INTR_FLAG_TIMER2
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	ands r0, r1, #INTR_FLAG_DMA0
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	ands r0, r1, #INTR_FLAG_DMA1
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	ands r0, r1, #INTR_FLAG_DMA2
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	ands r0, r1, #INTR_FLAG_DMA3
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	ands r0, r1, #INTR_FLAG_KEYPAD
	bne IntrMain_FoundIntr
	add r12, r12, 0x4
	@
	@ If the GAMEPAK interrupt is active, set SOUNDCNT_X to INTR_FLAG_GAMEPAK, 
	@ which clears all the bits in SOUNDCNT_X that actually mean anything, 
	@ and then jump back to the start of this subroutine and spin-loop.
	@
	ands r0, r1, #INTR_FLAG_GAMEPAK
	strbne r0, [r3, #REG_SOUNDCNT_X - REG_IE]
	bne . @ spin
IntrMain_FoundIntr:
	@
	@ Acknowledge the interrupt we are about to handle, and set r2 to the 
	@ bitmask of currently enabled interrupts excluding the interrupt we 
	@ are about to handle. We will use r2 to disable the handlers for low-
	@ priority interrupts, and for the current interrupt, until such time 
	@ as our current interrupt is handled.
	@
	strh r0, [r3, #OFFSET_REG_IF - 0x200]
	bic r2, r2, r0
	@
	@ gSTWIStatus->timerSelect indicates which of the four Timer interrupts 
	@ are being used by RFU/STWI (i.e. wireless link communications).
	@
	ldr r0, =gSTWIStatus
	ldr r0, [r0]
	ldrb r0, [r0, 0xA] @ Load gSTWIStatus->timerSelect.
	mov r1, 0x8
	lsl r0, r1, r0
	orr r0, r0, #INTR_FLAG_GAMEPAK
	orr r1, r0, #INTR_FLAG_SERIAL | INTR_FLAG_TIMER3 | INTR_FLAG_VCOUNT | INTR_FLAG_HBLANK
	and r1, r1, r2
	strh r1, [r3, #OFFSET_REG_IE - 0x200]
	@
	@ We have now disabled all interrupts except for GAMEPAK, SERIAL, TIMER3, 
	@ VCOUNT, HBLANK, and whichever timer is being used by RFU/STWI; and if 
	@ we are about to handle one of those, then that one is disabled as well. 
	@ We will re-enable them after we have finished handling the current 
	@ interrupt.
	@
	@ This is a simple priority system. Those interrupts are high-priority: 
	@ they can interrupt the handlers for the other, lower-priority, interrupts. 
	@ They cannot interrupt themselves, though, which ensures that if an 
	@ interrupt that occurs on a regular and frequent schedule (e.g. HBLANK) 
	@ has a handler that takes too long to run, the handler won't stack with 
	@ itself, snowball out of control, and block all other code from running.
	@
	mrs r3, cpsr
	bic r3, r3, #PSR_I_BIT | PSR_F_BIT | PSR_MODE_MASK
	orr r3, r3, #PSR_SYS_MODE
	msr cpsr_cf, r3
	@
	@ At last, we are ready to invoke the interrupt handler.
	@
	ldr r1, =gIntrTable
	add r1, r1, r12
	ldr r0, [r1]
	stmfd sp!, {lr}
	adr lr, IntrMain_RetAddr
	bx r0
IntrMain_RetAddr:
	ldmfd sp!, {lr}
	mrs r3, cpsr
	bic r3, r3, #PSR_I_BIT | PSR_F_BIT | PSR_MODE_MASK
	orr r3, r3, #PSR_I_BIT | PSR_IRQ_MODE
	msr cpsr_cf, r3
	ldmia sp!, {r0-r3,lr}
	strh r2, [r3, #OFFSET_REG_IE - 0x200]
	strh r1, [r3, #OFFSET_REG_IME - 0x200]
	msr spsr_cf, r0
	bx lr

	.pool

	.align 2, 0 @ Don't pad with nop.
