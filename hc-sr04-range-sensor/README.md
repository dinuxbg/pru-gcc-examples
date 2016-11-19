# hc-sr04-range-sensor

This is yet another HC-SR04 range sensor example for PRU. In an attempt to differentiate, it was written for the shiny new remoteproc/rpmsg driver, instead of the old uio_pruss.

To understand the rpmsg code here please visit http://processors.wiki.ti.com/index.php/PRU_Training:_Hands-on_Labs

## Wiring the sensor
There is a good explanation in https://github.com/HudsonWerks/Range-Sensor-PRU, so I'll just quote it:

> Hardware configuration:
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

## Building and running the example

First setup remoteproc driver by following [../REMOTEPROC.md](../REMOTEPROC.md).

Build and install firmware:

	cd hc-sr04-range-sensor
	make
	sudo cp out/pru-halt.elf /lib/firmware/am335x-pru0-fw
	sudo cp out/hc-sr04-range-sensor.elf /lib/firmware/am335x-pru1-fw
	sync
	reboot # Needed to load the firmware

To see the range measurement result in millimeters:

	sudo bash
	echo hello > /dev/rpmsg_pru31
	cat /dev/rpmsg_pru31   # Press Ctrl+C to exit

## Acknowledgements
 * Sensor idea and DTS from https://github.com/HudsonWerks/Range-Sensor-PRU
 * Remoteproc/RPMsg code from git://git.ti.com/pru-software-support-package/pru-software-support-package.git

