
## Setting up the remoteproc host driver

This is an attempt to document how to activate the PRU remoteproc driver on latest Debian distributions. For recent Debian versions, the steps are the same for both PRU GCC and for TI's toolchain. Feel free to raise a bug, or better yet - offer a pull request, if you notice any omissions.

Start by downloading, flashing and booting bone-debian-8.6-iot-armhf-2016-11-06-4gb.img.xz from https://beagleboard.org/latest-images .

If you need to use another kernel or distribution, please make sure to apply the remoteproc kernel fix for loading binutils PRU ELF:

	blinking-led/host-remoteproc/0001-Fix-remoteproc-to-work-with-the-PRU-GNU-Binutils-por.patch

Now a bit of manual work is required to enable remoteproc. Start by getting a prerequisite:

	git clone https://github.com/RobertCNelson/dtb-rebuilder
	cd ./dtb-rebuilder/

Edit your board file (either one of the following):

	nano src/arm/am335x-boneblack.dts
	nano src/arm/am335x-boneblack-overlay.dts
	nano src/arm/am335x-boneblack-emmc-overlay.dts
	nano src/arm/am335x-bonegreen.dts
	....
	nano src/arm/am335x-bone.dts

There, uncomment the following line:

	#include "am33xx-pruss-rproc.dtsi"

Then open this file:

	nano /etc/modprobe.d/pruss-blacklist.conf

And add a line to disable UIO:

	blacklist uio_pruss

Finally:

	make
	sudo make install
	sudo reboot

Finally, install your PRU ELF images. For example:

	cd blinking-led/pru
	make
	sudo cp out/pru-core0.elf /lib/firmware/am335x-pru0-fw
	sudo cp out/pru-core1.elf /lib/firmware/am335x-pru1-fw
	sync

There is an issue with remoteproc timing out waiting for PRU firmware on boot, simply because the root filesystem is mounted much later. Fix is simple - reload the remoteproc kernel module:

	sudo rmmod pru_rproc
	sudo modprobe pru_rproc
