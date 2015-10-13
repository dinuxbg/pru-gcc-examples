# blinking-led

## Firmware
To build the PRU firmware:

	cd blinking-led/pru
	make

## UIO host driver
The UIO loader has been tested with a "bone-debian-7.8-lxde-4gb-armhf-2015-03-01-4gb.img" image running 3.8 kernel.

To build the UIO-based firmware loader:

	apt-get install libelf-dev	# Needed by loader for parsing the ELF PRU executables
	cd blinking-led/host-uio
	make

Finally, to see a blinking led for 30 seconds on P9_27:

	modprobe uio_pruss
	echo BB-BONE-PRU-01 > /sys/devices/bone_capemgr.*/slots
	cd blinking-led/host
	./out/pload ../pru/out/pru-core0.elf ../pru/out/pru-core1.elf

## Remoteproc host driver

The example remoteproc driver has been taken from [beagleboard kernel branch 4.1](da821490ecd145e15d3aa3803c90ad8aef941eee), and patched with a minor fix to load ELF images from PRU Binutils.

First download, flash and boot bone-debian-8.2-console-armhf-2015-10-11-2gb.img .

Build and install firmware:

	cd blinking-led/pru
	make
	sudo cp out/pru-core0.elf /lib/firmware/am335x-pru0-fw
	sudo cp out/pru-core1.elf /lib/firmware/am335x-pru1-fw

Build and install the patched kernel module:

	sudo apt update
	sudo apt install build-essential linux-headers-`uname -r`
	cd blinking-led/host-remoteproc/
	make
	sudo cp pruss_remoteproc.ko /lib/modules/`uname -r`/kernel/drivers/remoteproc/pruss_remoteproc.ko
	sudo rmmod pruss_remoteproc
	sudo modprobe pruss_remoteproc

If you get kernel warnings then just reboot. This is a known issue.

In order to see the blinking led you'll need to configure the pin mux. Example for P9_27:

	sudo apt install git
	git clone https://github.com/beagleboard/bb.org-overlays.git
	cd bb.org-overlays
	./dtc-overlay.sh
	./install
	reboot
	sudo sh -c "echo 'cape-universal' > /sys/devices/platform/bone_capemgr/slots"
	sudo sh -c "echo 'pruout' > /sys/devices/platform/ocp/ocp:P9_27_pinmux/state"


## Acknowledgements
 * Parts of the AM33xx PRU package have been used for the UIO blinking LED example loader: https://github.com/beagleboard/am335x_pru_package
 * Beagleboard.org test debian image has been used for running the blinking LED example: http://beagleboard.org/latest-images/

