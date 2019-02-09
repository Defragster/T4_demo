
Debug_tt library :: Debug Teensy Trace

Enables Fault Trapping and Assert and user notice on Teensy.

Example of Usage for Asserts::  ASSERTS.INO
/* This example shows using assert_tt() to check values and HALT with indication 
 *  reference : https://en.wikipedia.org/wiki/Assert.h
 *  This is a macro that implements a runtime assertion, which can be used 
 *  to verify assumptions made by the program and print a diagnostic message
 *  if this assumption is false. When executed, if the expression is false
 *  that is, compares equal to 0), assert() will write information about 
 *  the call that failed on the output stream - USB by default
 *  If enabled it will fast toggle a pin - Teensy LED pin 13 by default
 *  
 *  This code has some assertions - commenting out one by one based on output
 *  will get to the next one until they are all gone.
 */  

Example of Usage for Fault trapping::  HARDFAULTS.INO
/* HardFaults demonstrates some found way to cause a processor Fault
 *  Normally these just HALT the processor and STOP all USB or other output
 *  Using the above included library maps the fault response to code that
 *  provides displayable feedback as demonstrated within.
 *  Some Faults are recoverable and continue, others repeat forever
 *  Those that repeat stop all user output and function, this library
 *  Keeps that output flowing where possible and FAST blinks the LED
 *  >> Allows AUTO programming of the Teensy without Button as the USB survives
 */



Example::  DebugTest
 is WIP for dev testing with both Assert and Fault creation together in some fashion. Other examples split them into shorter examples to see them independently.




-----
#include "debug_tt.h"
-----
Using library debug_tt in folder: t:\tcode\libraries\debug_tt (legacy)
Sketch uses 21044 bytes (2%) of program storage space. Maximum is 1048576 bytes.
Global variables use 5056 bytes (1%) of dynamic memory, leaving 257088 bytes for local variables. Maximum is 262144 bytes.


-----
#define DEBUG_OFF
#include "debug_tt.h"
-----
Using library debug_tt in folder: t:\tcode\libraries\debug_t3 (legacy)
Sketch uses 18192 bytes (1%) of program storage space. Maximum is 1048576 bytes.
Global variables use 4944 bytes (1%) of dynamic memory, leaving 257200 bytes for local variables. Maximum is 262144 bytes.

FUNCTIONS in keywords.txt::
----- conditional break of execution with display to SerMon and prompt
assert_tt( expression ) :: the expression is tested and if FALSE breaks in to the debug code for DISPLAY
haltif_tt( expression ) :: the expression is tested and if TRUE breaks in to the debug code for DISPLAY
----- displays to SerMon with a simple command to see
where_tt() :: Send a line of debug output to monitor showing:: Function name and line number and the address of a stack variable for reference
addr_tt(a) :: Shows the HEX memory address of the provided memory object
debText_tt(a) :: Prints the code line # and the value provided
debTextH_tt(a) :: Prints the code line # and the value provided shown in HEX
debText2_tt( a, b ) :: Prints the code line # and the two values provided
----- Logs to RAM for later display
deb_tt( a, b) :: Logs a setpoint in the storage array for the indicated value a and stores the uint32_t value, also records the line number and function
debBegin_tt :: Provides needed Begin values for the library
debTraceShow_tt :: Trace the trace log 
debTrace_tt :: Records a Trace entry
