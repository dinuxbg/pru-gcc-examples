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

#include <stdlib.h>
#include <stdint.h>

#include <pru/io.h>

#define PRU_OCP_RATE_HZ		(200 * 1000 * 1000)
#define PCLK_BIT		16	/* DO NOT CHANGE! */

#if defined(__AM335X_PRU0__)
  #define VSYNC_BIT		15
  #define HREF_BIT		14
  #define DATA_BIT		0
  #define PRU_CFG_GPCFG		PRU_CFG_GPCFG0
#elif defined(__AM335X_PRU1__)
  #define VSYNC_BIT		11
  #define HREF_BIT		10
  #define DATA_BIT		2
  #define PRU_CFG_GPCFG		PRU_CFG_GPCFG1
#else
  #error "Please configure your PRU connection."
#endif


/*
 * Calculate and return:
 * 	inp / (1 + 0.5 * divi)
 */
static long calc_gpo_div(long inp, long divi)
{
	return (inp * 2) / (2 + 1 * divi);
}

unsigned long ov7670_init(unsigned long xclk_rate_hz)
{
	long best_rate, div0, div1, best_div0, best_div1;

	/* Enable OCP access */
	PRU_CFG_SYSCFG.STANDBY_INIT = 0;

	/* Set input parallel capture mode for R31 */
	PRU_CFG_GPCFG.GPI_MODE = 1;
	PRU_CFG_GPCFG.GPI_CLK_MODE = 0;
	PRU_CFG_GPCFG.GPI_DIV0 = 0;
	PRU_CFG_GPCFG.GPI_DIV1 = 0;

	best_div1 = best_div0 = 0;
	best_rate = PRU_OCP_RATE_HZ;

	for (div0 = 0; div0 <= 0x1e; div0++) {
		for (div1 = 0; div1 <= 0x1e; div1++) {
			long rate, diff, best_diff;

			rate = calc_gpo_div(PRU_OCP_RATE_HZ, div0);
			rate = calc_gpo_div(rate, div1);
			diff = (xclk_rate_hz - rate);
			best_diff = (xclk_rate_hz - best_rate);

			if (labs(diff) < labs(best_diff)) {
				best_div0 = div0;
				best_div1 = div1;
				best_rate = rate;
			}
		}
	}

	/*
	 * Configure R30 in Shift Out Mode, which we'll use for
	 * generating the XCLK output. PRU_R30_0 (Shift Data) will
	 * be ignore, while PRU_R30_1 (Shift Clock) will clock OV7670.
	 */
	PRU_CFG_GPCFG.GPO_MODE = 1;
	PRU_CFG_GPCFG.GPO_DIV0 = best_div0;
	PRU_CFG_GPCFG.GPO_DIV1 = best_div1;

	/* Start shifting */
	write_r30((1u << 29) | 0xaaaa);
	write_r30(1u << 31);

	return best_rate;
}

/*
 * Wait for rising edge of pixel clock and read parallel output, including
 * both raw data and control lines.
 */
static inline uint32_t read_clocked_rawbyte(void)
{
	/* wait for LOW PCLK */
	while (read_r31() & (1u << PCLK_BIT));

	/* wait for HIGH PCLK */
	while (!(read_r31() & (1u << PCLK_BIT)));

	return read_r31();
}

uint32_t num_frames;

int ov7670_capture_frame(void *vmem, unsigned int xres, unsigned int yres,
			unsigned int stride)
{
	uint32_t x, y;
	uint8_t *line = vmem;

	/* wait for HIGH VSYNC */
	while (~read_clocked_rawbyte() & (1u << VSYNC_BIT));

	/* wait for LOW VSYNC */
	while (read_clocked_rawbyte() & (1u << VSYNC_BIT));

	for (y = 0; y < yres; y++) {
		const unsigned int line_nbytes = xres * 2;
		for (x = 0; x < line_nbytes; x += 4) {
			uint32_t data;
			asm volatile (
				/* fetch first byte (with HREF high) */
				"1:					\n\t"
				"qbbs	1b, r31, 16			\n\t"
				"11:					\n\t"
				"qbbc	11b, r31, 16			\n\t"
				"qbbc	1b, r31, %[href_bitn]		\n\t"
				"lsr	%[data].b0, r31, %[data_bitn]	\n\t"
				/* fetch second byte */
				"2:					\n\t"
				"qbbs	2b, r31, 16			\n\t"
				"22:					\n\t"
				"qbbc	22b, r31, 16			\n\t"
				"lsr	%[data].b1, r31, %[data_bitn]	\n\t"
				/* fetch third byte */
				"3:					\n\t"
				"qbbs	3b, r31, 16			\n\t"
				"33:					\n\t"
				"qbbc	33b, r31, 16			\n\t"
				"lsr	%[data].b2, r31, %[data_bitn]	\n\t"
				/* fetch fourth byte */
				"4:					\n\t"
				"qbbs	4b, r31, 16			\n\t"
				"44:					\n\t"
				"qbbc	44b, r31, 16			\n\t"
				"lsr	%[data].b3, r31, %[data_bitn]	\n\t"
				/* store the combined word to memory */
				"sbbo	%[data], %[line], %[line_offs], 4\n\t"
				: [data] "=r" (data)
				: [href_bitn] "L" (HREF_BIT),
				  [data_bitn] "L" (DATA_BIT),
				  [line_offs] "r" (x),
				  [line] "r" (line)
				: "memory");
		}
		line += stride;
	}

	num_frames++;
	return 0;
}

