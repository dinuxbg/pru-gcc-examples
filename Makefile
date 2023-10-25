error:
	@echo "Please build each example indivudually. Example:"
	@echo "   cd blinking-led/pru/"
	@echo "   make"
	@exit 1

# Check if all examples can be built.
# Leave simulator examples out of it - simulator is not
# usually packaged, and it's mainly for internal use anyway.
check-build:
	make -C blinking-led/pru/
	make -C blinking-led++/
	make -C button-blinking-led/pru/
	make -C hc-sr04-range-sensor/ clean
	make -C hc-sr04-range-sensor/ TISOC=am335x
	make -C hc-sr04-range-sensor/ clean
	make -C hc-sr04-range-sensor/ TISOC=tda4vm.icssg0
	make -C hc-sr04-range-sensor/ clean
	make -C hc-sr04-range-sensor/ TISOC=am62x
	make -C md5-check/pru/
	make -C ov7670-cam/pru

