# Example projects for the unofficial PRU-GCC port

The pru-gcc toolchain source and build instructions are located at https://github.com/dinuxbg/gnupru .

The following simple examples are available.

## blinking-led
The "hello world" example of the embedded world. Both PRU cores of a Beaglebone Black are started to toggle GPIOs. PRU0 core is programmed in C, while the firmware for PRU1 core is written in assembler.

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

## blinking-led++
Blinking led firmware written in C++.

## md5-check
Calculate MD5 checksum for a known data chunk on both the ARM loader and one PRU core. Then ARM loader reads and compares the two checksums.

## ov7670-cam
PRUs on a Beaglebone White fetch RGB565 stream from two OV7670 camera modules. The host-side loader then saves the images from the shared DDR-SDRAM memory into PPM files. The example shows how to access shared buffers in DDR-SDRAM, and how to write time-critical code sequences in inline assembly.

For information how to connect the OV7670 camera module to BeagleBone White, please see the included KiCad schematic. A cape PCB project is also included.

The example can work either with two cameras simultaneously, or, with some harmless warnings, only one camera module.

WARNING: For CAM0/PRU0 to work, BBW must be modified in order to expose some PRU0 pins. I have not tested BBB. BBW modification is not needed if only CAM1 will be used. BBW hardware change for CAM0 is as follows:
 * Remove R217 and solder R202. This exposes PRU0_R31_4 on P9_42.
 * Solder R221. This exposes PRU0_R31_6 on P9_41.

How to run:

	modprobe uio_pruss extram_pool_sz=2097152
	cp BB-BONE-OV7670-00A0.dtbo /lib/firmware/
	echo BB-BONE-OV7670 > /sys/devices/bone_capemgr.*/slots
	echo "BB-I2C1" > /sys/devices/bone_capemgr.*/slots
	./out/pload ../pru/out/pru-core0.elf ../pru/out/pru-core1.elf out0.ppm out1.ppm

Acknowledgements:
 * I2C initialization sequence comes from the OV7670 Linux kernel driver.
 * Cape schematics and PCB are slightly modified copies of FlyingBone ( https://github.com/piranha32/FlyingBone ).

Cape errata:
 * The two OV7670 pin header connectors are too close to each other. Two cameras can be plugged simultaneously only if one of them is slightly tilted.
 * Female long-pin headers are needed for the 2x23 connectors to BBW. Drills are 0.8mm, so the common male pin headers cannot fit.
 * There is no voltage buffer between BBW (3.3V I/O) and OV7670 (2.7V I/O). In theory this could damage the camera, but nevertheless it works for me (tm).
 * Some component markings are not visible on the final PCB because silk screen overlaps solder pads.

## sim-hello
Simulate your PRU executable on your PC. Note that pru-run is an ISA only simulator. Digital I/O is not simulated. Neither is OCP access.

## sim-hello++
Same simulator project as sim-hello, but written in C++ instead of C.
