EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:pru-ov7670-cape-cache
LIBS:beaglebone
LIBS:pru-ov7670-cape-cache
EELAYER 27 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 5
Title "PRU-OV7670-CAPE - Proto board for BeagleBone"
Date "29 mar 2015"
Rev "0.3"
Comp "Jacek Radzikowski <jacek.radzikowski@gmail.com>"
Comment1 "https://github.com/piranha32/FlyingBone"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
NoConn ~ 4350 3650
NoConn ~ 4350 3750
NoConn ~ 5750 3750
NoConn ~ 5750 3650
Text Label 5500 4600 0    60   ~ 0
BL_ISET2
Text Label 5500 4700 0    60   ~ 0
BL_OUT
Text Label 5500 4800 0    60   ~ 0
BL_SINK
Text Label 4700 4800 2    60   ~ 0
BL_SINK2
Text Label 4700 4700 2    60   ~ 0
BL_IN
Text Label 4700 4600 2    60   ~ 0
BL_ISET1
Wire Wire Line
	5750 4050 6050 4050
Wire Wire Line
	6050 4050 6050 4800
Wire Wire Line
	6050 4800 5500 4800
Wire Wire Line
	4350 3950 4150 3950
Wire Wire Line
	4150 3950 4150 4700
Wire Wire Line
	4150 4700 4700 4700
Wire Wire Line
	5500 4600 5850 4600
Wire Wire Line
	5850 4600 5850 3850
Wire Wire Line
	5850 3850 5750 3850
Wire Wire Line
	5500 4700 5950 4700
Wire Wire Line
	5950 4700 5950 3950
Wire Wire Line
	5950 3950 5750 3950
Wire Wire Line
	4350 3850 4250 3850
Wire Wire Line
	4250 3850 4250 4600
Wire Wire Line
	4250 4600 4700 4600
Wire Wire Line
	4350 4050 4050 4050
Wire Wire Line
	4050 4050 4050 4800
Wire Wire Line
	4050 4800 4700 4800
$Comp
L CONN_3X2 P2
U 1 1 4EB0F9FA
P 5100 4750
F 0 "P2" H 5100 5000 50  0000 C CNN
F 1 "Backl" V 5100 4800 40  0000 C CNN
F 2 "pin_array_3x2" H 5100 4750 60  0000 C CNN
F 3 "" H 5100 4750 60  0001 C CNN
	1    5100 4750
	-1   0    0    -1  
$EndComp
$Comp
L BEAGLEBONE U1
U 1 1 4EB0F922
P 4700 4150
F 0 "U1" H 4750 4100 60  0000 C CNN
F 1 "BEAGLEBONE" H 4950 4000 60  0000 C CNN
F 2 "BEAGLEBONE" H 4700 4150 60  0000 C CNN
F 3 "" H 4700 4150 60  0001 C CNN
	1    4700 4150
	1    0    0    -1  
$EndComp
$EndSCHEMATC
