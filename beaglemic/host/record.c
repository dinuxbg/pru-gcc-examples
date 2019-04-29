/* Copyright (c) 2018, Dimitar Dimitrov
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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#define RPMSG_BUF_SIZE	512

#define CMD_START	0x01
#define CMD_STOP	0x02

struct pru_start_rec {
	/* TODO - pass some options. */
	uint8_t cmd;
} __attribute__((__packed__));

struct pru_stop_rec {
	uint8_t cmd;
} __attribute__((__packed__));

static volatile bool exit_flag;

static void ctrl_c_signal_handler(int dummy __attribute__((unused)))
{
	exit_flag = true;
}

static void print_progress(unsigned int frcnt, ssize_t n)
{
	const int64_t NSECS_PER_MSEC = 1000000;
	const int64_t MSECS_PER_SEC = 1000;
	const int64_t UPDATE_MSECS = 100;
	static ssize_t total_bytes = 0;
	static int64_t t_last_print;

	int64_t t;
	struct timespec ts;

	total_bytes += n;
	clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
	t = (int64_t)ts.tv_sec * MSECS_PER_SEC + ts.tv_nsec / NSECS_PER_MSEC;
	if ((t - t_last_print) < UPDATE_MSECS)
		return;

	t_last_print = t;
	fprintf(stderr, "\rFrame: %-16u   (%zd:% 16zd)", frcnt, n, total_bytes);
	fflush(stderr);
}

static void stop_stream(int fd)
{
	struct pru_stop_rec rec;

	rec.cmd = CMD_STOP;
	write(fd, &rec, sizeof(rec));
}

/* You probably want to pipe to sox:
 *    $ ./record /dev/rpmsg_pru31 | sox -r 38110 -e signed -b 16 -c 16 -t raw - out.wav
 */
int main(int argc, char *argv[])
{
	struct pru_start_rec recopts = {0};
	const char *input_filename;
	int fd;

	if (argc == 1) {
		input_filename = "/dev/rpmsg_pru31";	/* the default */
	} else if (argc == 2) {
		input_filename = argv[1];
	} else {
		fprintf(stderr, "Invalid number of arguments given.\n");
		fprintf(stderr, "Usage: %s [/dev/rpmsg_pruX]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fd = open(input_filename, O_RDWR);
	if (fd < 0) {
		perror("Failed to open RPMSG device file");
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, ctrl_c_signal_handler);

	/* Initiate audio record. */
	recopts.cmd = CMD_START;
	write(fd, &recopts, sizeof(recopts));

	while (!exit_flag) {
		uint8_t buf[RPMSG_BUF_SIZE];
		unsigned int frcnt;
		ssize_t n = read(fd, buf, sizeof(buf));
		if (n < 0) {
			perror("RPMSG read failed");
			exit(EXIT_FAILURE);
		}
		if (n <= 4) {
			fprintf(stderr, "Short read from PRU: %zd\n", n);
			exit(EXIT_FAILURE);
		}
		/* Write data to stdout. */
		write(STDOUT_FILENO, buf, n - 4);

		/* Write frame counter to stderr. */
		frcnt = (unsigned long)buf[n - 4] << 0;
		frcnt |= (unsigned long)buf[n - 3] << 8;
		frcnt |= (unsigned long)buf[n - 2] << 16;
		frcnt |= (unsigned long)buf[n - 1] << 24;
		print_progress(frcnt, n - 4);
	}
	stop_stream(fd);
	close(fd);

	return EXIT_SUCCESS;
}
