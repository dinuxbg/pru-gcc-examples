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
	.section .init0
	.global	__start
__start:
	/* Initialize stack pointer. */
	ldi	sp.w2, %hi(__stack_top)
	ldi	sp.w0, %lo(__stack_top)
	jmp	main

	.text
	.section .text
main:
	ldi	r0, 0
	ldi	r1, 0xffff

main_loop:
	ldi	r14.w2, %hi(CPU_HZ / 4)
	ldi	r14.w0, %lo(CPU_HZ / 4)
	call	delay_n2_cycles
	/* DING */
	mov	r30, r0
	ldi	r14.w2, %hi(CPU_HZ / 4)
	ldi	r14.w0, %lo(CPU_HZ / 4)
	call	delay_n2_cycles
	/* DONG */
	mov	r30, r1
	jmp main_loop

delay_n2_cycles:
	sub	r14, r14, 1
	qbne	delay_n2_cycles, r14, 0
	ret


	/* Dummy data, required by remoteproc loader */
	.data
	.section .resource_table,"aw",@progbits
my_resource_table:
	.word	1, 0, 0, 0	/* struct resource_table base */
	.word	0		/* uint32_t offset[1] */
