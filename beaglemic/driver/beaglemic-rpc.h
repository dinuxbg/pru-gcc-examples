/*
 * RPC definitions, shared with PRU firmware.
 *
 * Copyright (C) 2019 Dimitar Dimitrov <dimitar@dinux.eu>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define BEAGLEMIC_PCM_SAMPLE_RATE	32000
#define BEAGLEMIC_PCM_NCHANNELS		16

#define BEAGLEMIC_PRUCMD_PREPARE	0x01
#define BEAGLEMIC_PRUCMD_START		0x02
#define BEAGLEMIC_PRUCMD_STOP		0x03

#define RPMSG_BEAGLEMIC_CHAN_NAME	"rpmsg-beaglemic"

struct beaglemic_pru_prepare_rec {
	uint8_t cmd;	/* keep always the first member. */
	uint8_t channels;

	uint32_t buffer_addr;
	uint32_t buffer_nbytes;

	uint32_t period_size;

	/* TODO - pass other options. */
} __attribute__((__packed__));

struct beaglemic_pru_simple_command {
	uint8_t cmd;	/* keep always the first member. */
} __attribute__((__packed__));

struct beaglemic_pru_status {
	uint32_t hwptr;		/* byte offset from beginning of buffer. */
	uint32_t err;		/* Zero for no issues, otherwise an error code. */
	uint32_t frame_counter;
} __attribute__((__packed__));
