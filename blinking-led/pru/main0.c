/* Copyright (c) 2014, Dimitar Dimitrov
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

#include <pru/io.h>

#include "resource_table_0.h"

static void delay_us(unsigned int us)
{
	/* assume cpu frequency is 200MHz */
	__delay_cycles (us * (1000 / 5));
}

const unsigned int period_us = 250 * 1000;
#define GPIO1_BASE		0x4804C000

#define GPIO1_OE		(*(volatile uint32_t *)(GPIO1_BASE + 0x134))
#define GPIO1_DATAIN		(*(volatile uint32_t *)(GPIO1_BASE + 0x138))
#define GPIO1_CLEARDATAOUT	(*(volatile uint32_t *)(GPIO1_BASE + 0x190))
#define GPIO1_SETDATAOUT	(*(volatile uint32_t *)(GPIO1_BASE + 0x194))
#define TRIG_BIT	24

int main(void)
{
	unsigned int c;

	/* allow OCP master port access by the PRU so the PRU
	 * can read external memories */
	//CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;
	PRU_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	GPIO1_OE &= ~(1u << TRIG_BIT);	/* output */
	for (c = 0; ; c++) {
		write_r30(c & 1 ? 0xffff : 0x0000);
		if (c & 1)
			GPIO1_SETDATAOUT = 1u << TRIG_BIT;
		else
			GPIO1_CLEARDATAOUT = 1u << TRIG_BIT;

		delay_us (period_us);
	}

	return 0;
}
