/* main0.S - TDM acquisition frontend.
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

#include "common.h"

#define REG_DOUT32	r25		/* Fake output data. */
#define REG_FRCNT	r24		/* Frame counter. Conveniently placed
					   immediately after PCM output regs. */

#define INTC_COMPOSITE_SYS_BASE	16
#define PRU0_TO_PRU1_INTR	(21 - INTC_COMPOSITE_SYS_BASE)
#define R31_INTR_STROBE		(1u << 5)

#define R30_TDM_DOUT_PIN	14
#define R30_DOUT_EN_PIN		15

#define R31_TDM_BCLK_PIN	0
#define R31_TDM_FCLK_PIN	1
#define R31_TDM_DIN_PIN		2

.altmacro

.macro	wait_bclk_high
	qbbc	.+0, r31, R31_TDM_BCLK_PIN
.endm

.macro	wait_bclk_low
	qbbs	.+0, r31, R31_TDM_BCLK_PIN
.endm

.macro	handle_dbit	RIN, ROUT, BIT
	wait_bclk_low
	/* Set output */
	qbbc	.+8, \ROUT, \BIT
	set	r30, r30, R30_TDM_DOUT_PIN
	qbbs	.+8, \ROUT, \BIT
	clr	r30, r30, R30_TDM_DOUT_PIN
	wait_bclk_high
	/* Read output */
	qbbc	.+8, r31, R31_TDM_DIN_PIN
	set	\RIN, \RIN, \BIT
	qbbs	.+8, r31, R31_TDM_DIN_PIN
	clr	\RIN, \RIN, \BIT
.endm

.macro rxtx_16bit	RIN, ROUT
	handle_dbit	\RIN, \ROUT, 15
	handle_dbit	\RIN, \ROUT, 14
	handle_dbit	\RIN, \ROUT, 13
	handle_dbit	\RIN, \ROUT, 12
	handle_dbit	\RIN, \ROUT, 11
	handle_dbit	\RIN, \ROUT, 10
	handle_dbit	\RIN, \ROUT, 9
	handle_dbit	\RIN, \ROUT, 8
	handle_dbit	\RIN, \ROUT, 7
	handle_dbit	\RIN, \ROUT, 6
	handle_dbit	\RIN, \ROUT, 5
	handle_dbit	\RIN, \ROUT, 4
	handle_dbit	\RIN, \ROUT, 3
	handle_dbit	\RIN, \ROUT, 2
	handle_dbit	\RIN, \ROUT, 1
	handle_dbit	\RIN, \ROUT, 0
.endm

.macro rxtx_32bit	RIN, ROUT
	rxtx_16bit	\RIN\().w2, \ROUT\().w2
	rxtx_16bit	\RIN\().w0, \ROUT\().w0
.endm

	.text
	.section .init0, "x"
	.global	_start
_start:
	ldi	REG_DOUT32, 0
	ldi	REG_FRCNT, 0

	/* TODO - drive this dynamically! */
	set	r30, r30, R30_DOUT_EN_PIN
	jmp	main_loop

#if TDM_SLOT_SIZE != 32
  #error "Please revise"
#endif

#if TDM_NUM_SLOTS != 2
  #error "Please revise"
#endif

	.text
	.section .text
main_loop:
	/* Wait for Frame low->high transition.
	 * WARNING: Assuming sync offset is 1.
	 */
	/* TODO - here we rely on a few extra slots
	 * so that we can catch the Frame Edge and
	 * do some transfers.
	 */
wait_frame_high:
	wait_bclk_low
	wait_bclk_high
	qbbc	wait_frame_high, r31, R31_TDM_FCLK_PIN

	rxtx_32bit	r16, REG_DOUT32

	/* Output triangle wave. */
	add	REG_DOUT32.w2, REG_DOUT32.w2, 1
	/* Increment frame counter. */
	add	r24, r24, 1

	rxtx_32bit	r17, REG_DOUT32

        /* Write to peer. */
        xout    SCRATCH_BANK_2, r16, 8*4 + 4
        /* Ping peer. */
        ldi     r31, R31_INTR_STROBE | PRU0_TO_PRU1_INTR

	/* Wait for Frame high->low transition.
	 * WARNING: Assuming sync offset is 1.
	 */
wait_frame_low:
	wait_bclk_low
	wait_bclk_high
	qbbs	wait_frame_low, r31, R31_TDM_FCLK_PIN

	jmp	main_loop


	/* Dummy data, required by remoteproc loader */
	.data
	.section .resource_table,"aw",@progbits
my_resource_table:
	.word	1, 0, 0, 0	/* struct resource_table base */
	.word	0		/* uint32_t offset[1] */
