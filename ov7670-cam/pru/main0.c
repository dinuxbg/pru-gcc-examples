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
#include <string.h>
#include <pru/io.h>

#include "ov7670.h"

/* Filled-in by the host with video memory pointer. */
void *video_memory;

int main(void)
{
	if (!video_memory)
		for (;;) ;

	/* Hardcode VGA for the time being.
	 *
	 *   frames per second ---------------------------------.
	 *   two bytes per pixel ---------------------------.   |
	 *   PCK = XCK / 2   ---------------------------.   |   |
	 *   vertical resolution ---.                   |   |   |
	 *   xres  --.              |                   |   |   |
	 *           |              |                   |   |   |  */
	ov7670_init((640 + 144) * (480 + 3 + 17 + 10) * 2 * 2 * 15);

	memset(video_memory, 0x00, 640 * 480 * 2);

	for (;;) {
		ov7670_capture_frame(video_memory, 640, 480, 640 * 2);
	}

	return EXIT_SUCCESS;
}
