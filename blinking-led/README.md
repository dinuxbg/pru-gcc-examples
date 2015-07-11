# blinking-led
To build the PRU firmware:

	cd blinking-led/pru
	make

Then, to build the UIO-based firmware loader:

	apt-get install libelf-dev	# Needed by loader for parsing the ELF PRU executables
	cd blinking-led/host
	make

Finally, to see a blinking led for 30 seconds on P9_27:

	modprobe uio_pruss
	echo BB-BONE-PRU-01 > /sys/devices/bone_capemgr.*/slots
	cd blinking-led/host
	./out/pload ../pru/out/pru-core0.elf ../pru/out/pru-core1.elf

Acknowledgements:
 * Parts of the AM33xx PRU package have been used for the blinking LED example loader: https://github.com/beagleboard/am335x_pru_package
 * Beagleboard.org test debian image has been used for running the blinking LED example: http://beagleboard.org/latest-images/

