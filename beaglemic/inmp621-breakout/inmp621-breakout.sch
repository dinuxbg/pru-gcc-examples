EESchema Schematic File Version 4
LIBS:inmp621-breakout-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "INMP621 PDM Microphone Breakout"
Date "2020-01-04"
Rev "1.0-rc1"
Comp "dimitar@dinux.eu"
Comment1 "NOT YET VALIDATED!"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Device:C C1
U 1 1 5E07B3DD
P 3300 2250
F 0 "C1" H 3415 2296 50  0000 L CNN
F 1 "1u" H 3415 2205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 3338 2100 50  0001 C CNN
F 3 "~" H 3300 2250 50  0001 C CNN
	1    3300 2250
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 5E07B443
P 3750 2250
F 0 "C2" H 3865 2296 50  0000 L CNN
F 1 "100n" H 3865 2205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 3788 2100 50  0001 C CNN
F 3 "~" H 3750 2250 50  0001 C CNN
	1    3750 2250
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x05 J1
U 1 1 5E07B4D3
P 5050 2300
F 0 "J1" H 5130 2342 50  0000 L CNN
F 1 "Conn_01x05" H 5130 2251 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x05_P2.54mm_Vertical" H 5050 2300 50  0001 C CNN
F 3 "~" H 5050 2300 50  0001 C CNN
	1    5050 2300
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5E07B5D2
P 1050 2450
F 0 "R1" H 1120 2496 50  0000 L CNN
F 1 "NC" H 1120 2405 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 980 2450 50  0001 C CNN
F 3 "~" H 1050 2450 50  0001 C CNN
	1    1050 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 2100 3300 2100
Wire Wire Line
	3300 2100 3750 2100
Connection ~ 3300 2100
Wire Wire Line
	4850 2100 3750 2100
Connection ~ 3750 2100
Wire Wire Line
	4050 2200 4850 2200
Wire Wire Line
	1050 2200 1900 2200
Wire Wire Line
	1050 2200 1050 2300
Wire Wire Line
	1050 2600 1050 2950
Wire Wire Line
	1050 2950 3300 2950
Wire Wire Line
	4850 2300 4650 2300
Text Label 4650 2300 0    50   ~ 0
SEL
Text Label 4650 2200 0    50   ~ 0
GND
Text Label 4650 2100 0    50   ~ 0
3V3
Wire Wire Line
	4850 2400 4650 2400
Wire Wire Line
	4850 2500 4650 2500
Text Label 4650 2400 0    50   ~ 0
CLK
Text Label 4650 2500 0    50   ~ 0
DAT
Wire Wire Line
	1900 2300 1700 2300
Wire Wire Line
	1900 2400 1700 2400
Text Label 1700 2200 0    50   ~ 0
SEL
Text Label 1700 2300 0    50   ~ 0
CLK
Text Label 1700 2400 0    50   ~ 0
DAT
$Comp
L INMP621:INMP621 MK1
U 1 1 5E09082C
P 2350 2300
F 0 "MK1" H 2350 2767 50  0000 C CNN
F 1 "INMP621" H 2350 2676 50  0000 C CNN
F 2 "inmp621-footprint:MK_INMP621" H 2350 2300 50  0001 L BNN
F 3 "Warning" H 2350 2300 50  0001 L BNN
F 4 "LGA-5 TDK-InvenSense" H 2350 2300 50  0001 L BNN "Поле4"
F 5 "INMP621" H 2350 2300 50  0001 L BNN "Поле5"
F 6 "Mic Omni-Directional -46dB 3.63V Rectangular Solder Pad" H 2350 2300 50  0001 L BNN "Поле6"
F 7 "TDK-InvenSense" H 2350 2300 50  0001 L BNN "Поле7"
F 8 "None" H 2350 2300 50  0001 L BNN "Поле8"
	1    2350 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3300 2400 3300 2950
Wire Wire Line
	2800 2400 2900 2400
Connection ~ 3300 2400
Wire Wire Line
	2800 2500 2900 2500
Wire Wire Line
	2900 2500 2900 2400
Connection ~ 2900 2400
Wire Wire Line
	2900 2400 3300 2400
Wire Wire Line
	3750 2400 4050 2400
Wire Wire Line
	4050 2400 4050 2200
Wire Wire Line
	3300 2400 3750 2400
Connection ~ 3750 2400
$EndSCHEMATC
