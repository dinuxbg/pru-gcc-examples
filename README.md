# Example projects for the unofficial PRU-GCC port

The pru-gcc toolchain source and build instructions are located at https://github.com/dinuxbg/gnupru .

The following simple examples are available. Check the README.md in each subdirectory for detailed instructions.

## blinking-led
The "hello world" example of the embedded world. Both PRU cores of a Beaglebone Black are started to toggle GPIOs. PRU0 core is programmed in C, while the firmware for PRU1 core is written in assembler.

Two host loader examples are provided - one for the old UIO mechanism, and one for the new Remoteproc driver.

## blinking-led++
Blinking led firmware written in C++.

## button-blinking-led
Button-controlled blinking LED. Shows both basic input and basic output.

## hc-sr04-range-sensor
Remoteproc/rpmsg example for measuring distance using the HC-SR04 ultrasound range sensor.The rpmsg communication happens between PRU1 and ARM core.In order to perfrom communication between PRU0 and ARM core.</br>

- Patch as following</br>
	patch -s -p0 < hc-sr04-range-sensorPRU0.patch


- To restore the changes made by patch</br>
	patch -p0 -R -i hc-sr04-range-sensorPRU0.patch

## md5-check
Calculate MD5 checksum for a known data chunk on both the ARM loader and one PRU core. Then UIO-based loader reads and compares the two checksums.

## ov7670-cam
PRUs on a Beaglebone White fetch RGB565 streams from two OV7670 camera modules. The UIO host-side loader then saves the images from the shared DDR-SDRAM memory into PPM files. The example shows how to access shared buffers in DDR-SDRAM, and how to write time-critical code sequences in inline assembly.

## sim-hello
Simulate your PRU executable on your PC. Note that pru-run is an ISA only simulator. Digital I/O is not simulated. Neither is OCP access.

## sim-hello++
Same simulator project as sim-hello, but written in C++ instead of C.

## External Projects
Some examples are too complex to pile up in this repository, yet could be useful as a reference.
  * [BeagleMic Microphone Array](https://gitlab.com/dinuxbg/beaglemic)
