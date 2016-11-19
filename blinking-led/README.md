# blinking-led

## Firmware
To build the PRU firmware:

	cd blinking-led/pru
	make

## UIO host driver

There is a nice automated script to configure [the latest Beaglebone Debian image](https://debian.beagleboard.org/images/bone-debian-8.4-lxqt-4gb-armhf-2016-05-13-4gb.img.xz) to enable the UIO host driver.

	git clone https://github.com/Neil-Jubinville/pru
	cd pru
	./prep_pru.sh

## Remoteproc host driver

First setup remoteproc driver by following [../REMOTEPROC.md](../REMOTEPROC.md).

Build and install firmware:

	cd blinking-led/pru
	make
	sudo cp out/pru-core0.elf /lib/firmware/am335x-pru0-fw
	sudo cp out/pru-core1.elf /lib/firmware/am335x-pru1-fw
	sync

In order to see the blinking led you'll need to configure the pin mux. Example for P9_27:

	sudo sh -c "echo 'pruout' > /sys/devices/platform/ocp/ocp:P9_27_pinmux/state"


## Acknowledgements
 * Parts of the AM33xx PRU package have been used for the UIO blinking LED example loader: https://github.com/beagleboard/am335x_pru_package
 * Beagleboard.org test debian image has been used for running the blinking LED example: http://beagleboard.org/latest-images/

