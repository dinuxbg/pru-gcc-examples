/*
 * HC-SR04 remoteproc/rpmsg project based on TI's lab05 example.
 *
 * Copyright (C) 2015 Dimitar Dimitrov <dinuxbg@gmail.com>
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

#include "../driver/beaglemic-rpc.h"

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


#define CHAN_DESC			"Channel 31"
#define CHAN_PORT			31

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK	4

char payload[RPMSG_BUF_SIZE];
struct {
	/* RPMSG channel settings. */
	uint16_t src;
	uint16_t dst;

	/* Is stream initialized by host (i.e. is it running). */
	bool initialized;

	uint32_t dma_addr;
	uint32_t dma_bytes;
	uint32_t period_size;
	uint32_t dma_i;
} stream;

static void prepare_stream(struct beaglemic_pru_prepare_rec *prep)
{
	stream.dma_addr = prep->buffer_addr;
	stream.dma_bytes = prep->buffer_nbytes;
	stream.period_size = prep->period_size;
	stream.dma_i = 0;
}

static void handle_host_interrupt(struct pru_rpmsg_transport *transport)
{
	uint16_t src, dst, len;

	/* Clear the event status */
	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

	/* Receive all available messages, multiple messages can be sent per kick */
	while (pru_rpmsg_receive(transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS) {
		switch (payload[0]) {
		case BEAGLEMIC_PRUCMD_PREPARE:
			stream.src = src;
			stream.dst = dst;
			stream.initialized = false;
			if (len == sizeof(struct beaglemic_pru_prepare_rec))
				prepare_stream((struct beaglemic_pru_prepare_rec *) payload);
			break;
		case BEAGLEMIC_PRUCMD_START:
			stream.initialized = true;
			break;
		case BEAGLEMIC_PRUCMD_STOP:
			stream.initialized = false;
			break;
		default:
			/* TODO - handle errors. */
			break;
		}
	}
}

/*
 * Get the next audio frame from peer PRU and store it
 * at buffer bufptr. The number of written bytes is:
 *    16 channels with * 2 bytes for each = 32 bytes
 *
 * Return the current frame counter value.
 */
static inline unsigned int acquire_frame_from_peer(void *bufptr)
{
	unsigned int fcntr;

	asm volatile (
		"xin	%[SCRATCH_BANK], r16, 16*2 + 4		\n\t"
		"sbbo	r16, %[bufptr], 0, 16*2			\n\t"
		/* Keep as the last instruction, [fcntr] does not
		 * have the & constraint modifier. */
		"mov	%[fcntr], r24				\n\t"
		: [fcntr] "=r" (fcntr)
		: [bufptr] "r" (bufptr), [SCRATCH_BANK] "i" (SCRATCH_BANK_2)
		: "memory", "r16", "r17", "r18", "r19",
		  "r20", "r21", "r22", "r23", "r24");

	return fcntr;

}

static void handle_peer_interrupt(struct pru_rpmsg_transport *transport)
{
	unsigned int frame_counter;

	/* Clear the event status */
	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_PEER;

	if (!stream.dma_addr || !stream.dma_bytes || !stream.period_size)
		return;

	frame_counter = acquire_frame_from_peer((void *)(stream.dma_addr + stream.dma_i));
	stream.dma_i += 16 * 2;

	if (stream.dma_i >= stream.dma_bytes)
		stream.dma_i = 0;

	if ((stream.dma_i % stream.period_size) == 0) {
		struct beaglemic_pru_status hwst;

		hwst.hwptr = stream.dma_i;
		hwst.err = 0;	/* TODO */
		hwst.frame_counter = frame_counter;

		/* Send HW pointer IRQ to host. */
		pru_rpmsg_send(transport, stream.dst,
			       stream.src,
			       &hwst, sizeof(hwst));
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
	while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, RPMSG_BEAGLEMIC_CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS)
		;

	while (1) {
		/* Check bit 31 of register R31 to see
		 * if the mailbox interrupt has occurred */
		if (read_r31() & HOST_INT) {
			handle_host_interrupt(&transport);
		}
		if (stream.initialized && (read_r31() & PEER_INT)) {
			handle_peer_interrupt(&transport);
		}
	}

	return 0;
}
