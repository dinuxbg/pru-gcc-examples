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

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/i2c-dev.h>

#include "ov7670-defs.h"

static int echo(const char *file, const char *val)
{
	FILE *f = fopen(file, "w");
	if (!f) {
		perror("could not open GPIO sysfs control file");
		return -errno;
	}
	fwrite(val, strlen(val), 1, f);
	fclose(f);
	return 0;
}

static int ov7670_reset(const int gpio_n)
{
	const char *sysfs_dir = "/sys/class/gpio";
	char path[256];
	char val[256];
	int st;

	path[sizeof(path) - 1] = '\0';
	val[sizeof(val) - 1] = '\0';

	snprintf(path, sizeof(path) - 1, "%s/export", sysfs_dir);
	snprintf(val, sizeof(val) - 1, "%d", gpio_n);
	st = echo(path, val);
	if (st)
		return st;

	snprintf(path, sizeof(path) - 1, "%s/gpio%d/direction",
			sysfs_dir, gpio_n);
	st = echo(path, "out");
	if (st)
		return st;

	snprintf(path, sizeof(path) - 1, "%s/gpio%d/value",
			sysfs_dir, gpio_n);
	st = echo(path, "0");
	if (st)
		return st;

	usleep(50 * 1000);

	st = echo(path, "1");

	/* datasheet says 1ms, but just to be safe */
	usleep(10 * 1000);

	return st;
}

static int ov7670_write_regs(int fd, struct ov7670_reg *regs)
{
	for (; regs->addr != 0xff && regs->val != 0xff; regs++) {
		uint8_t buf[2];
		
		buf[0] = regs->addr;
		buf[1] = regs->val;
		if (write(fd, buf, 2) != 2) {
			fprintf(stderr, "attempted to set [0x%02x]=0x%02x\n",
					regs->addr, regs->val);
			perror("could not write OV7670 I2C register");
			return -errno;
		}
	}

	return 0;
}

int ov7670_i2c_setup(const int busid, const int i2c_addr, const int reset_gpio)
{
	int fd;
	int ret;
	char filename[256];

	if (ov7670_reset(reset_gpio))
		return -EIO;

	snprintf(filename, sizeof(filename) - 1, "/dev/i2c-%d", busid);
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		perror("could not open i2c adapter");
		return -errno;
	}

	if (ioctl(fd, I2C_SLAVE, i2c_addr) < 0) {
		perror("could not set I2C slave address");
		return -errno;
	}

	ret = ov7670_write_regs(fd, ov7670_default_regs);
	if (ret)
		return ret;

	ret = ov7670_write_regs(fd, ov7670_fmt_rgb565);
	if (ret)
		return ret;

	close(fd);
	return 0;
}

