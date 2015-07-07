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
Sheet 1 2
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
U 1 1 555CAB78
P 2800 3150
F 0 "U?" H 2950 3450 70  0000 C CNN
F 1 "TL081" H 2950 3350 70  0000 C CNN
F 2 "" H 2800 3150 60  0000 C CNN
F 3 "" H 2800 3150 60  0000 C CNN
	1    2800 3150
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR?
U 1 1 555CAC8F
P 2700 2650
F 0 "#PWR?" H 2700 2610 30  0001 C CNN
F 1 "+3.3V" H 2700 2760 30  0000 C CNN
F 2 "" H 2700 2650 60  0000 C CNN
F 3 "" H 2700 2650 60  0000 C CNN
	1    2700 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	2700 2650 2700 2750
Wire Wire Line
	1050 3050 2300 3050
$Comp
L GND #PWR?
U 1 1 555CAD7E
P 2700 3600
F 0 "#PWR?" H 2700 3600 30  0001 C CNN
F 1 "GND" H 2700 3530 30  0001 C CNN
F 2 "" H 2700 3600 60  0000 C CNN
F 3 "" H 2700 3600 60  0000 C CNN
	1    2700 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2700 3550 2700 3600
Wire Wire Line
	2300 3250 2150 3250
Wire Wire Line
	2150 3250 2150 3850
Wire Wire Line
	2150 3850 3400 3850
Wire Wire Line
	3400 3850 3400 3150
Wire Wire Line
	3300 3150 3400 3150
Wire Wire Line
	3400 3150 4100 3150
Connection ~ 3400 3150
Text GLabel 1050 3050 0    60   Input ~ 0
input
Text GLabel 4100 3150 2    60   Output ~ 0
output
$Sheet
S 8300 1550 1300 600 
U 555CAF7B
F0 "Subsheet" 50
F1 "example_subsheet.sch" 50
$EndSheet
$EndSCHEMATC
