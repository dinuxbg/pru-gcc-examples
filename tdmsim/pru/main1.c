/*
 * Copyright (C) 2018 Dimitar Dimitrov <dinuxbg@gmail.com>
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 	* Redistributions of source code must retain the above copyright
 * 	  notice, this list of conditions and the following disclaimer.
 *
 * 	* Redistributions in binary form must reproduce the above copyright
 * 	  notice, this list of conditions and the following disclaimer in the
 * 	  documentation and/or other materials provided with the
 * 	  distribution.
 *
 * 	* Neither the name of Texas Instruments Incorporated nor the names of
 * 	  its contributors may be used to endorse or promote products derived
 * 	  from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_virtqueue.h>
#include <pru_rpmsg.h>
#include "resource_table_1.h"

#include <pru/io.h>

#include "common.h"

/* Host-1 Interrupt sets bit 31 in register R31 */
#define HOST_INT			((uint32_t) 1 << 31)
/* Peer PRU Interrupt sets bit 30 in register R31 */
#define PEER_INT			((uint32_t) 1 << 30)

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 */
#define TO_ARM_HOST			18
#define FROM_ARM_HOST			19

#define FROM_PEER			21


#define CHAN_NAME			"rpmsg-pru"

#define CHAN_DESC			"Channel 31"
#define CHAN_PORT			31

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK	4

char payload[RPMSG_BUF_SIZE];
struct {
	uint16_t src;
	uint16_t dst;
	bool initialized;
} rpmsg_settings;

static void handle_host_interrupt(struct pru_rpmsg_transport *transport)
{
	uint16_t src, dst, len;

	/* Clear the event status */
	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

	/* Receive all available messages, multiple messages can be sent per kick */
	while (pru_rpmsg_receive(transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS) {
		if (!rpmsg_settings.initialized) {
			rpmsg_settings.src = src;
			rpmsg_settings.dst = dst;
			rpmsg_settings.initialized = true;
		}

		/* TODO - send runtime init config to PRU peer. */
	}
}

#define N_SHARED_REGS	8
#if ((TDM_SLOT_SIZE * TDM_NUM_SLOTS + 7) / 8) > (N_SHARED_REGS * 4)
 #error "Please revise the register layout"
#endif

/*
 * Get the next audio frame from peer PRU and store it
 * at buffer bufptr.
 *
 * Return the current frame counter value.
 */
static inline unsigned int acquire_frame_from_peer(void *bufptr)
{
	unsigned int fcntr;

	asm volatile (
		"xin	%[SCRATCH_BANK], r16, %[nbytes] + 4	\n\t"
		"sbbo	r16, %[bufptr], 0, %[nbytes]		\n\t"
		/* Keep as the last instruction, [fcntr] does not
		 * have the & constraint modifier. */
		"mov	%[fcntr], r24				\n\t"
		: [fcntr] "=r" (fcntr)
		: [bufptr] "r" (bufptr), [SCRATCH_BANK] "i" (SCRATCH_BANK_2),
		  [nbytes] "i" ((TDM_SLOT_SIZE * TDM_NUM_SLOTS + 7) / 8)
		: "memory", "r16", "r17", "r18", "r19",
		  "r20", "r21", "r22", "r23", "r24");

	return fcntr;

}

static void handle_peer_interrupt(struct pru_rpmsg_transport *transport)
{
	static int buflen;
	static char pcmbuf[RPMSG_BUF_SIZE];
	unsigned int frame_counter;

	/* Clear the event status */
	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_PEER;

	frame_counter = acquire_frame_from_peer(&pcmbuf[buflen]);
	buflen += 16 * 2;

	if (buflen == (16 * 2 * 15)) {
		pcmbuf[buflen + 0] = frame_counter >> 0;
		pcmbuf[buflen + 1] = frame_counter >> 8;
		pcmbuf[buflen + 2] = frame_counter >> 16;
		pcmbuf[buflen + 3] = frame_counter >> 24;

		/* 16*2*15 + 4 = 484 bytes per message. */
                pru_rpmsg_send(transport, rpmsg_settings.dst,
			       rpmsg_settings.src,
                               pcmbuf, buflen + 4);
		buflen = 0;
	}
}

int main(void)
{
	struct pru_rpmsg_transport transport;
	volatile uint8_t *status;

	/* allow OCP master port access by the PRU so the PRU
	 * can read external memories */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	/* clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

	/* Make sure the Linux drivers are ready for RPMsg communication */
	status = &resourceTable.rpmsg_vdev.status;
	while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK))
		;

	/* Initialize the RPMsg transport structure */
	pru_rpmsg_init(&transport,
		       &resourceTable.rpmsg_vring0,
		       &resourceTable.rpmsg_vring1,
		       TO_ARM_HOST,
		       FROM_ARM_HOST);

	/* Create the RPMsg channel between the PRU and ARM user space using the transport structure. */
	while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS)
		;

	while (1) {
		/* Check bit 31 of register R31 to see
		 * if the mailbox interrupt has occurred */
		if (read_r31() & HOST_INT) {
			handle_host_interrupt(&transport);
		}
		if (rpmsg_settings.initialized && (read_r31() & PEER_INT)) {
			handle_peer_interrupt(&transport);
		}
	}

	return 0;
}
