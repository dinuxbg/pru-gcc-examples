
## Introduction

If you have a recent Beagleboard Debian image, then stop reading - remoteproc should already be enabled and patched for pru-gcc!

If you have flashed the latest Beagleboard Debian but remoteproc is still not working for you, then please make a post in https://groups.google.com/g/beagleboard


## Setting Up The Remoteproc Host Driver

Continue reading if you need to use non-beagleboard kernel or distribution.

### Applying Patch For GNU Binutils

Make sure to apply the remoteproc kernel fix for loading binutils PRU ELF:

	blinking-led/host-remoteproc/0001-Fix-remoteproc-to-work-with-the-PRU-GNU-Binutils-por.patch

### Disabling UIO

Disable UIO to avoid interfering with remoteproc. Open this file on the target:

	nano /etc/modprobe.d/pruss-blacklist.conf

And add a line to disable UIO:

	blacklist uio_pruss

Don't forget to reboot so that settings take effect.

### Device Tree
Specifics depend on the particular kernel because TI has different remoteproc versions for each kernel release they make.
