# Example projects for the unofficial PRU-GCC port

The pru-gcc toolchain source and build instructions are located at https://github.com/dinuxbg/gnupru .

The following simple examples are available.

## blinking-led
The "hello world" example of the embedded world. Both PRU cores of a Beaglebone Black are started to toggle GPIOs. PRU0 core is programmed in C, while the firmware for PRU1 core is written in assembler.

## md5-check
Calculate MD5 checksum for a known data chunk on both the ARM loader and one PRU core. Then ARM loader reads and compares the two checksums.

## sim-hello
Simulate your PRU executable on your PC. Note that pru-run is an ISA only simulator. Digital I/O is not simulated. Neither is OCP access.


