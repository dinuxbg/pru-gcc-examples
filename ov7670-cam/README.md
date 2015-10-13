# ov7670-cam
For information how to connect the OV7670 camera module to BeagleBone White, please see the included KiCad schematic. A cape PCB project is also included.

The example can work either with two cameras simultaneously, or, with some harmless warnings, only one camera module.

WARNING: For CAM0/PRU0 to work, BBW must be modified in order to expose some PRU0 pins. I have not tested BBB. BBW modification can be skipped if using only CAM1/PRU1. BBW hardware change for CAM0 is as follows:
 * Remove R217 and solder R202. This exposes PRU0_R31_4 on P9_42.
 * Solder R221. This exposes PRU0_R31_6 on P9_41.

How to run:

	modprobe uio_pruss extram_pool_sz=2097152
	cp BB-BONE-OV7670-00A0.dtbo /lib/firmware/
	echo BB-BONE-OV7670 > /sys/devices/bone_capemgr.*/slots
	echo "BB-I2C1" > /sys/devices/bone_capemgr.*/slots
	cd host-uio
	./out/pload ../pru/out/pru-core0.elf ../pru/out/pru-core1.elf out0.ppm out1.ppm

Acknowledgements:
 * I2C initialization sequence comes from the OV7670 Linux kernel driver.
 * Cape schematics and PCB are slightly modified copies of FlyingBone ( https://github.com/piranha32/FlyingBone ).

Cape errata:
 * The two OV7670 pin header connectors are too close to each other. Two cameras can be plugged simultaneously only if one of them is slightly tilted.
 * Female long-pin headers are needed for the 2x23 connectors to BBW. Drills are 0.8mm, so the common male pin headers cannot fit.
 * There is no voltage buffer between BBW (3.3V I/O) and OV7670 (2.7V I/O). In theory this could damage the camera, but nevertheless it works for me (tm).
 * Some component markings are not visible on the final PCB because silk screen overlaps solder pads.

