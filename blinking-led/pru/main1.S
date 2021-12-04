/* main1.S - blinking LED example
 *
 * Copyright (c) 2014 Dimitar Dimitrov <dinuxbg@gmail.com>
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

/* PRU clock rate */
#define CPU_HZ	(200 * 1000 * 1000)

	.text
	.section .init0, "x"
	.global	_start
_start:
	/* Initialize stack pointer. */
	ldi32	sp, _stack_top
	jmp	main

	.text
	.section .text
main:
	ldi	r0, 0
	ldi	r1, 0xffff

main_loop:
	ldi32	r14, CPU_HZ / 4
	call	delay_n2_cycles
	/* DING */
	mov	r30, r0
	ldi32	r14, CPU_HZ / 4
	call	delay_n2_cycles
	/* DONG */
	mov	r30, r1
	jmp main_loop

delay_n2_cycles:
	sub	r14, r14, 1
	qbne	delay_n2_cycles, r14, 0
	ret
