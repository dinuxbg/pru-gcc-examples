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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "image.h"

int save_image_rgb(void *image, unsigned int xres, unsigned int yres,
		unsigned int stride, const char *filename)
{
	uint8_t *b, *line;
	unsigned int x, y;
	FILE *outf;

	if (0) {
		outf = fopen("out.raw", "wb");
		fwrite(image, xres * yres * 2, 1, outf);
		fclose(outf);
	}
	/* PPM format */
	outf = fopen(filename, "wb");
	if (!outf)
		return -ENOENT;

	fprintf(outf, "P6\n%d %d\n255\n", xres, yres);
	for (y = 0, line = image; y < yres; y++, line += stride) {
		for (x = 0, b = line; x < xres; x++, b += 2) {
			/* RGB565 */
			unsigned int red = (b[0] >> 3) & 0x1f;
			unsigned int green = ((b[0] & 0x7) << 3)
						| ((b[1] >> 5) & 0x7);
			unsigned int blue = b[1] & 0x1f;
			fputc(red << 3 , outf);
			fputc(green << 2, outf);
			fputc(blue << 3, outf);
		}
	}

	fclose(outf);

	return 0;
}

/*
 * Source:
 * https://thinksmallthings.wordpress.com/2012/11/03/ov7670-yuv-demystified/comment-page-1/
 */
int save_image_yuv(void *image, unsigned int xres, unsigned int yres,
		unsigned int stride, const char *filename)
{
	uint8_t *b, *line;
	unsigned int x, y;
	FILE *outf;

	/* PPM format */
	outf = fopen(filename, "wb");
	if (!outf)
		return -ENOENT;

	fprintf(outf, "P6\n%d %d\n255\n", xres, yres);
	for (y = 0, line = image; y < yres; y++, line += stride) {
		for (x = 0, b = line; x < xres; x += 2, b += 4) {
			const int Y  = b[0];
			const int U  = b[1];
			const int Y1 = b[2];
			const int V  = b[3];
			int R, G, B;

			R = Y + 14075 * (V - 128);
			G = Y - 3455 * (U - 128) - (7169 * (V - 128));
			B = Y + 17790 * (U - 128);
			fputc(R / 10000, outf);
			fputc(G / 10000, outf);
			fputc(B / 10000, outf);

			R = Y1 + 14075 * (V - 128);
			G = Y1 - 3455 * (U - 128) - (7169 * (V - 128));
			B = Y1 + 17790 * (U - 128);
			fputc(R / 10000, outf);
			fputc(G / 10000, outf);
			fputc(B / 10000, outf);
		}
	}

	fclose(outf);

	return 0;
}

