
Test Package:
lps_test.exe : for Windows - runs to receive 100 blocks of 32KB
 >> Param1 :: serial port like 'Com25'
 >> Param2 :: 0 for 100000000 block loops
 >> Param2 :: 1 for 1000000 block loops
 >> Param2 :: 2-9 for # of 100 block loops
OUTPUT :: reads COM# block of data and shows one string per block

lps_test.c : source for precompiled EXE - could take mods for non-Windows

bld2.bat : cmdline compile for gcc 

EchoBoth : Sketch for second Teensy. 
 >> Echos Serial1 input at 115200 baud
 >> Read FreqCount from pin13

TelemViewFast : sketch for T4 to demo Lines Per Second output
 >> Serial2 connects 115200
 >> pinMode(14, OUTPUT); // Toggle w/each output string for FreqCount
 >> uint32_t usDelay = 10; // start us delay between transmit
 >> //#define DELAY_DROP 1 // comment to keep a static delay


Usage: Wire Serial as noted above, and FreqCount pin
Put Echoboth on a Teensy
Put TelemViewFast on Target Teensy
From Windows cmdline run for device port:: lps_test COM25

OPTIONAL: when working the output is formatted for TelemetryViewer graphing. Download the JAR from 'http://www.farrellf.com/TelemetryViewer/'  and get it running with proper JAVA runtime. 'Layout' is in file : "RealTest20.txt"