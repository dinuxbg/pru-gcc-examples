/* main0.S - PDM acquisition frontend.
 *
 * Copyright (c) 2018 Dimitar Dimitrov <dimitar@dinux.eu>
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

/*
 * Native bank:
 *   r0.w0, r1.w0, r2.w0, ..., r15.w0		Integrator 1
 *   r0.w2, r1.w2, r2.w2, ..., r15.w2		Integrator 2
 *   r16.w0, r16.w2, r17.w0, ..., r23.w2	Temporary for PCM transfer
 *
 * Bank 0:
 *   r0.w0, r1.w0, r2.w0, ..., r15.w0		Comb 1, delay1
 *   r0.w2, r1.w2, r2.w2, ..., r15.w2		Comb 1, delay2
 *
 * Bank 1:
 *   r0.w0, r1.w0, r2.w0, ..., r15.w0		Comb 2, delay1
 *   r0.w2, r1.w2, r2.w2, ..., r15.w2		Comb 2, delay2
 *
 * Bank 2:
 *   r16.w0, r16.w2, r17.w0, ..., r23.w2	Comb 2, pcmout
 *   r24					Comb 2, frame counter
 *
 * Warning: XCHG instruction is not supported on AM335x. See:
 *   http://e2e.ti.com/support/processors/f/791/t/340589?Am335x-PRU-scratchpad-XCHG-problem-
 *
 *
 * Notes:
 *  - The parallel 8-bit input data is strobed. The 9 cycles of COMB
 *    processing is not enough to fit the data setup time of the MIC
 *    output.
 */

#include "common.h"

#define RPDATA	r29.b1		/* Parallel 8-bit input strobe. */
#define RJMPSUB r29.w2		/* Subroutine - the next stage. */
#define RTMP	r28.w0		/* Temporary value. */
#define RSCALE	r28.w2		/* Stores value 0x7fff. */
#define RSAVE	r27		/* Store saved register value. */

#define RFRCNT	r24		/* Frame counter. Conveniently placed
				   immediately after PCM output regs. */

#define INTC_COMPOSITE_SYS_BASE	16
#define PRU0_TO_PRU1_INTR	(21 - INTC_COMPOSITE_SYS_BASE)
#define R31_INTR_STROBE		(1u << 5)

#define R30_CLK_PIN		14

.altmacro

/* Total: 4 cycles */
.macro	handle_dbit y1, y2, bin, rin
	qbbc	.+8, \rin, \bin
	add	\y1, \y1, 1
	qbbs	.+8, \rin, \bin
	sub	\y1, \y1, 1
	add	\y2, \y2, \y1
.endm

/* Total: 8*4=32 cycles. */
.macro	handle_data8l
	handle_dbit r0.w0, r0.w2, 0, RPDATA
	handle_dbit r2.w0, r2.w2, 1, RPDATA
	handle_dbit r4.w0, r4.w2, 2, RPDATA
	handle_dbit r6.w0, r6.w2, 3, RPDATA
	handle_dbit r8.w0, r8.w2, 4, RPDATA
	handle_dbit r10.w0, r10.w2, 5, RPDATA
	handle_dbit r12.w0, r12.w2, 6, RPDATA
	handle_dbit r14.w0, r14.w2, 7, RPDATA
.endm

.macro	handle_data8r
	handle_dbit r1.w0, r1.w2, 0, RPDATA
	handle_dbit r3.w0, r3.w2, 1, RPDATA
	handle_dbit r5.w0, r5.w2, 2, RPDATA
	handle_dbit r7.w0, r7.w2, 3, RPDATA
	handle_dbit r9.w0, r9.w2, 4, RPDATA
	handle_dbit r11.w0, r11.w2, 5, RPDATA
	handle_dbit r13.w0, r13.w2, 6, RPDATA
	handle_dbit r15.w0, r15.w2, 7, RPDATA
.endm

/* Comb1 stage: 9 cycles. 
 *
 * Record the output into RTMP. This implies that the next
 * subroutine _must_ be the comb2 stage for the same
 * MIC channel.
 *
 * Note: INZ comb work register number must equal the
 * native register number used by the same MICN
 * integrator.
 */
.macro	__comb1_subroutine	MICN, INZ
comb1_mic\MICN:
	/* Save the integrator working register. 
	 * It also contains _our_ Integrator 2 output! */
	mov	RSAVE, r\INZ

	xin	SCRATCH_BANK_1, r\INZ, 4
	sub	RTMP, RSAVE.w2, r\INZ\().w2
	mov	r\INZ\().w2, r\INZ\().w0
	mov	r\INZ\().w0, RSAVE.w2
	xout	SCRATCH_BANK_1, r\INZ, 4

	/* Restore integrator working register. */
	mov	r\INZ, RSAVE
	ldi	RJMPSUB, %pmem(comb2_mic\MICN)
	jmp	main_loop
.endm

/* Comb2 stage: 9 cycles.
 *
 * Requires input from previous stage into RTMP.
 * Output to COUT.
 */
.macro	__comb2_subroutine	MICN, C2OUT, C2OUTWS, OUTZ
comb2_mic\MICN:
	/* Save the integrator working register. */
	mov	RSAVE, r\OUTZ

	xin	SCRATCH_BANK_2, r\OUTZ, 4
	sub	r\C2OUT\().w\C2OUTWS, RTMP, r\OUTZ\().w2
	mov	r\OUTZ\().w2, r\OUTZ\().w0
	mov	r\OUTZ\().w0, RTMP
	xout	SCRATCH_BANK_2, r\OUTZ, 4

	/* Restore integrator working register. */
	mov	r\OUTZ, RSAVE
	ldi	RJMPSUB, %pmem(comb2_mic\MICN\()_next_stage)
	jmp	main_loop
comb2_mic\MICN\()_next_stage:
.endm

.macro	comb_subroutines	MICN
	__comb1_subroutine MICN, %(0 + MICN)
	__comb2_subroutine MICN, %(16 + MICN/2), %(2 * (MICN % 2)), %(0 + MICN)
.endm

/* Idle stage: 9 cycles. */
.macro	idle_subroutine
idle_subroutine_\@:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	ldi	RJMPSUB, %pmem(idle_subroutine_\@_end)
	jmp	main_loop
idle_subroutine_\@_end:
.endm
	.text
	.section .init0, "x"
	.global	_start
_start:
	ldi	RJMPSUB, %pmem(subroutine_dispatch_table_start)
	ldi	RPDATA, 0
	ldi	RFRCNT, 0
	jmp	main_loop

	.text
	.section .text
main_loop:
	
	/* ---------- CLK 0 --------------------------------------*/
	mov	RPDATA, r31			/* 1 */
	clr	r30, r30, R30_CLK_PIN		/* 1 */
	handle_data8l				/* 32 = 4 * 8 */
	nop					/* 1 -> EXTRA 1 */
	nop					/* 1 -> EXTRA 1 */
	nop					/* 1 -> EXTRA 1 */
	nop					/* 1 -> EXTRA 1 */
	nop					/* 1 -> EXTRA 1 */
	nop					/* 1 */
	nop					/* 1 */
	nop					/* 1 */
	nop					/* 1 */
	nop					/* 1 */
	nop					/* 1 */
	nop					/* 1 */
	nop					/* 1 */
	nop					/* 1 */
	nop					/* 1 */

	/* ---------- CLK 1 --------------------------------------*/
	mov	RPDATA, r31			/* 1 */
	set	r30, r30, R30_CLK_PIN		/* 1 */
	nop					/* 1 -> EXTRA 1 */
	nop					/* 1 -> EXTRA 1 */
	nop					/* 1 -> EXTRA 1 */
	nop					/* 1 -> EXTRA 1 */
	nop					/* 1 -> EXTRA 1 */
	handle_data8r				/* 32 = 4 * 8 */
	jmp RJMPSUB				/* 1 jmp, 9 subroutine cycles*/


/*
 * Dispatch table with short subroutines, which are executed during
 * the few spare cycles we have between handling PDM bit data.
 * We use them to run the comb filters.
 */
subroutine_dispatch_table_start:
	/*
	 * 16 microphones, with 2 comb stages each, totalling
	 * 32 subroutine stages.
	 */
	.set i,0
	.rept 16
	   comb_subroutines	%i
	   .set i,i+1
	.endr
	/* 31 idle (nop) stages */
	.set i,0
	.rept 31
	   idle_subroutine
	   .set i,i+1
	.endr

	/* Stage to send results to peer PRU (9 cycles). */
	nop
	nop
	nop
	nop
	/* Increment frame counter. */
	add	r24, r24, 1
	/* Write to peer. */
	xout    SCRATCH_BANK_2, r16, 8*4 + 4
	/* Ping peer. */
	ldi	r31, R31_INTR_STROBE | PRU0_TO_PRU1_INTR
	ldi	RJMPSUB, %pmem(subroutine_dispatch_table_start)
	jmp	main_loop


	/* Dummy data, required by remoteproc loader */
	.data
	.section .resource_table,"aw",@progbits
my_resource_table:
	.word	1, 0, 0, 0	/* struct resource_table base */
	.word	0		/* uint32_t offset[1] */
