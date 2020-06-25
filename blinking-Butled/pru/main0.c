/*  Copyright (c) 2020, Code https://github.com/dinuxbg/pru-gcc-examples/blob/master/blinking-led/pru/main0.c
 *  by  Dimitar Dimitrov is  modified by Deepankar maithani to include a button and targeting a specific pin  as output and another one as input
 *  using  bit masking
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


/*
 * IF on POCKET BEAGLE:
 * Connect LED to 2.34
 * Connect Button to 2.32
 *
 * IF on BEAGLEBONE BLACK OR WIRELESS
 * connect LED to P9.27
 * Connect Button to 9.30
 *
 *
 * The Button connection Diagram   3.3v-------___ ------------------|----------------------------^^^^------GND
 *                                             |
 *                         			 		                        ^
 *                         			 		                        |
 *                         			 		 Button          connect wire from mark           1k ohm
 *                          		 		 not pressed     to P2.32						  resistance
 *
 *      When button is not pushed  the P2.32 is connected to GND when button is pressed there is voltage drop across resistance and 3.3V is applied to P2.32
 *
 *
 */

#include <pru/io.h>
#include <stdint.h>

#include "resource_table_0.h"

#define LED ((uint32_t) 1 << 5)
#define BUTTON			((uint32_t) 1 << 2)


static void delay_us(unsigned int us)
{
	/* assume cpu frequency is 200MHz */
	__delay_cycles (us * (1000 / 5));
}

const unsigned int period_us = 250 * 1000;


int main(void)

{
	unsigned int c;
	while(1)	{



			if(read_r31() &  BUTTON	)  // Check if  the Button is Pressed or not. By reading the r31 and masking using button bit

				{
					// the value of c 0000 (0), 0001(1),0010(2),0011(3),0100(4) and so on till 1001(9). so last bit is toggling between 0 and 1.
					//LED blinks 4 Times
					for(c=0;c<9;c++)
						{
							write_r30(c & 1 ? (read_r30()|LED) : ~(LED)&read_r30()); //      (setbit : clearbit)
							delay_us (period_us);
						}
				}

}



	return 0;

}



