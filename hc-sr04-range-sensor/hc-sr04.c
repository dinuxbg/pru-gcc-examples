/* Copyright (c) 2015, Dimitar Dimitrov
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <pru/io.h>

#include "hc-sr04.h"

#define PRU_OCP_RATE_HZ		(200 * 1000 * 1000)

#define TRIG_PULSE_US		10

#define GPIO1_BASE		0x4804C000

#define GPIO1_OE		(*(volatile uint32_t *)(GPIO1_BASE + 0x134))
#define GPIO1_DATAIN		(*(volatile uint32_t *)(GPIO1_BASE + 0x138))
#define GPIO1_CLEARDATAOUT	(*(volatile uint32_t *)(GPIO1_BASE + 0x190))
#define GPIO1_SETDATAOUT	(*(volatile uint32_t *)(GPIO1_BASE + 0x194))

#define TRIG_BIT		12
#define ECHO_BIT		13

void hc_sr04_init(void)
{
        /* Enable OCP access */
        PRU_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	/*
	 * Don't bother with PRU GPIOs. Our timing requirements allow
	 * us to use the "slow" system GPIOs.
	 */
	GPIO1_OE &= ~(1u << TRIG_BIT);	/* output */
	GPIO1_OE |= (1u << ECHO_BIT);	/* input */
}

int hc_sr04_measure_pulse(void)
{
	bool echo, timeout;

	/* pulse the trigger for 10us */
	GPIO1_SETDATAOUT = 1u << TRIG_BIT;
	__delay_cycles(TRIG_PULSE_US * (PRU_OCP_RATE_HZ / 1000000));
	GPIO1_CLEARDATAOUT = 1u << TRIG_BIT;

	/* Enable counter */
	PRU_CTRL.CYCLE = 0;
	PRU_CTRL.CONTROL_bit.COUNTER_ENABLE = 1;

	/* wait for ECHO to get high */
	do {
		echo = !!(GPIO1_DATAIN & (1u << ECHO_BIT));
		timeout = PRU_CTRL.CYCLE > PRU_OCP_RATE_HZ;
	} while (!echo && !timeout);

	PRU_CTRL.CONTROL_bit.COUNTER_ENABLE = 0;

	if (timeout)
		return -1;

	/* Restart the counter */
	PRU_CTRL.CYCLE = 0;
	PRU_CTRL.CONTROL_bit.COUNTER_ENABLE = 1;

	/* measure the "high" pulse length */
	do {
		echo = !!(GPIO1_DATAIN & (1u << ECHO_BIT));
		timeout = PRU_CTRL.CYCLE > PRU_OCP_RATE_HZ;
	} while (echo && !timeout);

	PRU_CTRL.CONTROL_bit.COUNTER_ENABLE = 0;

	if (timeout)
		return -1;

	uint64_t cycles = PRU_CTRL.CYCLE;

	return cycles / ((uint64_t)PRU_OCP_RATE_HZ / 1000000);
}
