# hc-sr04-range-sensor

This is yet another HC-SR04 range sensor example for PRU. In an attempt to differentiate, it was written for the shiny new remoteproc/rpmsg driver, instead of the old uio_pruss.

To understand the rpmsg code here please visit http://processors.wiki.ti.com/index.php/PRU_Training:_Hands-on_Labs

:warning: **WARNING**: This version requires remoteproc driver from Linux kernel version 5.10 or later, as well as RPMSG driver for PRU. For earlier kernels, please check the other branches of this project.

## 1. Build and install
First step is to build the firmware and install it. Export the `TISOC` variable with the SoC variant for your board.

	export TISOC=am335x          # For BeagleBone "classics"
	export TISOC=tda4vm.icssg0   # For Beaglebone-AI64
	export TISOC=am62x           # For BeaglePlay

	cd hc-sr04-range-sensor
	make
	sudo bash
	cp out/pru-halt.elf /lib/firmware/
	cp out/hc-sr04-range-sensor.elf /lib/firmware/

## 2. Setup on Beaglebone Black/Green/White
### 2.1. Wiring the sensor
There is a good explanation in https://github.com/HudsonWerks/Range-Sensor-PRU, so I'll just quote it:

> Hardware configuration
> 
>         * TRIGGER               P8_12 gpio1[12] GPIO44  out     pulldown                Mode: 7 
>         * ECHO                  P8_11 gpio1[13] GPIO45  in      pulldown                Mode: 7 *** with R 1KOhm
>         * GND                   P9_1 or P9_2    GND
>         * VCC                   P9_5 or P9_6    VDD_5V
>         
> Schematic:
>         
> ![ch6_pru_range_sensor](https://cloud.githubusercontent.com/assets/4622940/8599064/4d14cb26-262c-11e5-9c46-1961dc67bdcc.png)
> 
> NOTE: The resistors are important. Since the sensor emits a 5V signal, and the Beaglebone Black's input pins are only 3.3V, using resistors or voltage converters is crucial for preventing damage to your board.

### 2.2. Starting the firmware

	echo pru-halt.elf > /sys/class/remoteproc/remoteproc1/firmware
	echo hc-sr04-range-sensor.elf > /sys/class/remoteproc/remoteproc2/firmware
	echo start > /sys/class/remoteproc/remoteproc1/state
	echo start > /sys/class/remoteproc/remoteproc2/state

## 3. Setup on BeagleBone-AI64

Wire as follows:

	* TRIGGER               P9_13   gpio0_2
	* ECHO                  P9_11   gpio0_1     **** with R 1KOhm!!!
	* GND                   P9_1 or P9_2      GND
	* VCC                   P9_7 or P9_8      VDD_5V

Pin mux for those pins should already default to GPIO mode if you are running the BeagleBoard Debian images.

Find the PRU1 instance for remoteproc. It was `remoteproc3` for me, but it might be different for you:

	cat /sys/class/remoteproc/remoteproc3/name
	-> b038000.pru

Start the HC-SR04 firmware:

	echo hc-sr04-range-sensor.elf > /sys/class/remoteproc/remoteproc3/firmware
	echo start > /sys/class/remoteproc/remoteproc3/state

## 4. Setup on BeaglePlay

Wire as follows:

	* TRIGGER               MK10    RST    gpio1_12
	* ECHO                  MK7     INT    gpio1_9     **** with R 1KOhm!!!
	* GND                   MK1     GND
	* VCC                   MK2     +5V

Pin mux for those pins should already default to GPIO mode if you are running the BeagleBoard Debian images.

Find the PRU1 instance for remoteproc. It was `remoteproc1` for me, but it might be different for you:

	cat /sys/class/remoteproc/remoteproc1/name
	30078000.pru

Start the HC-SR04 firmware:

	echo hc-sr04-range-sensor.elf > /sys/class/remoteproc/remoteproc1/firmware
	echo start > /sys/class/remoteproc/remoteproc1/state

## 5. Triggering a measurement

To see the range measurement result in millimeters:

	sudo bash
	echo hello > /dev/rpmsg_pru31
	cat /dev/rpmsg_pru31   # Press Ctrl+C to exit

Or, alternatively, as a one-liner in BASH:

	echo s >/dev/rpmsg_pru31; dd if=/dev/rpmsg_pru31 bs=32 count=1 2>/dev/null; echo

## Acknowledgements
 * Sensor idea and DTS from https://github.com/HudsonWerks/Range-Sensor-PRU
 * Remoteproc/RPMsg code from git://git.ti.com/pru-software-support-package/pru-software-support-package.git

