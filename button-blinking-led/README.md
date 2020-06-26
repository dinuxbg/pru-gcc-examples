# button-blinking-led
This example is written to demonstrate how to use input and control output based on the input value.

##Wiring the Button and LED

![PocketArcade_bb](https://user-images.githubusercontent.com/6142587/85893431-27bf3600-b7f3-11ea-91e9-f82e7ed2346a.png)
## Firmware
To build the PRU firmware:

	cd blinking-led/pru
	make

## Remoteproc host driver

First setup remoteproc driver by following [../REMOTEPROC.md](../REMOTEPROC.md).

Build and install firmware:

	cd blinking-led/pru
	make
	sudo cp out/pru-core0.elf /lib/firmware/am335x-pru0-fw
	sudo cp out/pru-core1.elf /lib/firmware/am335x-pru1-fw
	sync

In order to see the blinking led when you press the button you'll need to configure the pin mux:

       
	sudo sh -c "echo 'pruout' > /sys/devices/platform/ocp/ocp:P2_34_pinmux/state"
	sudo sh -c "echo 'pruin' > /sys/devices/platform/ocp/ocp:P2_32_pinmux/state"

or you can also use the following commands  to configure pin mux:

	sudo config-pin P2.34 pruout
	sudo config-pin P2.32 pruin

change the pin numbers as per your usage

## Acknowledgements
 * Parts of the AM33xx PRU package have been used for the UIO blinking LED example loader: https://github.com/beagleboard/am335x_pru_package
 * Beagleboard.org test debian image has been used for running the blinking LED example: http://beagleboard.org/latest-images/

