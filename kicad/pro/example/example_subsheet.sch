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
LIBS:yeolab_customparts
EELAYER 27 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 2
Title ""
Date "20 may 2015"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L TL081 U?
U 1 1 555CB00D
P 3300 2850
F 0 "U?" H 3450 3150 70  0000 C CNN
F 1 "TL081" H 3450 3050 70  0000 C CNN
F 2 "" H 3300 2850 60  0000 C CNN
F 3 "" H 3300 2850 60  0000 C CNN
	1    3300 2850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 2400 3200 2450
Wire Wire Line
	3200 3250 3200 3300
$Comp
L R R?
U 1 1 555CB08B
P 2400 2950
F 0 "R?" V 2480 2950 40  0000 C CNN
F 1 "R" V 2407 2951 40  0000 C CNN
F 2 "~" V 2330 2950 30  0000 C CNN
F 3 "~" H 2400 2950 30  0000 C CNN
	1    2400 2950
	0    -1   -1   0   
$EndComp
$Comp
L R R?
U 1 1 555CB0A3
P 3200 3700
F 0 "R?" V 3280 3700 40  0000 C CNN
F 1 "R" V 3207 3701 40  0000 C CNN
F 2 "~" V 3130 3700 30  0000 C CNN
F 3 "~" H 3200 3700 30  0000 C CNN
	1    3200 3700
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2650 2950 2800 2950
Wire Wire Line
	2750 2950 2750 3700
Wire Wire Line
	2750 3700 2950 3700
Connection ~ 2750 2950
Wire Wire Line
	3450 3700 3850 3700
Wire Wire Line
	3850 3700 3850 2850
Wire Wire Line
	3800 2850 4200 2850
Connection ~ 3850 2850
$Comp
L GND #PWR?
U 1 1 555CB0CA
P 2750 2800
F 0 "#PWR?" H 2750 2800 30  0001 C CNN
F 1 "GND" H 2750 2730 30  0001 C CNN
F 2 "" H 2750 2800 60  0000 C CNN
F 3 "" H 2750 2800 60  0000 C CNN
	1    2750 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 2750 2750 2750
Wire Wire Line
	2750 2750 2750 2800
$Comp
L +5V #PWR?
U 1 1 555CB0FC
P 3200 2400
F 0 "#PWR?" H 3200 2490 20  0001 C CNN
F 1 "+5V" H 3200 2490 30  0000 C CNN
F 2 "" H 3200 2400 60  0000 C CNN
F 3 "" H 3200 2400 60  0000 C CNN
	1    3200 2400
	1    0    0    -1  
$EndComp
$Comp
L -5V #PWR?
U 1 1 555CB10B
P 3200 3300
F 0 "#PWR?" H 3200 3440 20  0001 C CNN
F 1 "-5V" H 3200 3410 30  0000 C CNN
F 2 "" H 3200 3300 60  0000 C CNN
F 3 "" H 3200 3300 60  0000 C CNN
	1    3200 3300
	-1   0    0    1   
$EndComp
Wire Wire Line
	2150 2950 1900 2950
Text HLabel 1900 2950 0    60   Input ~ 0
input
Text HLabel 4200 2850 2    60   Output ~ 0
output
$EndSCHEMATC
