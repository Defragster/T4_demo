// #define DEBUG_OFF // #define DEBUG_OFF
#include "debug_tt.h"

/* This example shows using assert_tt() to check values and HALT with indication
    reference : https://en.wikipedia.org/wiki/Assert.h
    This is a macro that implements a runtime assertion, which can be used
    to verify assumptions made by the program and print a diagnostic message
    if this assumption is false. When executed, if the expression is false
    that is, compares equal to 0), assert() will write information about
    the call that failed on the output stream - USB by default
    If enabled it will fast toggle a pin - Teensy LED pin 13 by default

    This code has some assertions - commenting out one by one based on output
    will get to the next one until they are all gone.
*/

void setup() {
  Serial1.begin( 115200 );
  Serial1.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  Serial.begin(115200);
  while (!Serial && millis() < 5000 );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  if ( !DebState( LED_BUILTIN ) ) {
    Serial.println("\n Not FAULTED.");
  }

  DebText2( "Hello World ", micros() );
  Serial.printf( "Hello World %u", micros() );

  deb_tt( 7, micros() );
  float x = log (0);
  where_tt();
  DebText( "ping" );
  deb_tt( 3, millis() );
  haltif_tt( isfinite( x ) ); // Assure float not NAN of INF
  haltif_tt( 0 ); // Assure float not NAN of INF
  deb_tt( micros(), 4 );
  assert_tt( isfinite( x ) ); // Assure float not NAN of INF
  deb_tt( 8, F_CPU );
  assert_tt( !isinf( x ) );

  deb_tt( 5, 0x101010 );
  x = 0.0 / 0.0;
  // IMXRT_BUGBUG_wtf  assert_tt( !isnan( 1.0 ) );
  deb_tt( 8, F_CPU );
  // IMXRT_BUGBUG_wtf  assert_tt( !isnan( x ) );

  digitalWrite(LED_BUILTIN, 0);

  deb_tt( 9, 7 );
  debug_fault( 0 );
  // IMXRT_BUGBUG_wtf  assert_tt( true ); // Assert this exppression is TRUE - no Assert
  deb_tt( 9, 42 );
  // IMXRT_BUGBUG_wtf  assert_tt( false ); // Assert this exppression is TRUE - will Halt
  deb_tt( 401, 0 ); // Display information kept on deb_tt() calls

  Serial.println("\n DONE! All Good");
}

void loop() {
  delay(1000);
  qBlink();
}

#ifdef __cplusplus
extern "C" {
#endif
#include "debug/printf.h"
#undef printf
extern void systick_isr(void);
extern volatile uint32_t systick_millis_count;
#define printf_debug Serial1.printf

//__attribute__((naked))
void unused_interrupt_vector(void)
{
  __asm( ".syntax unified\n"
         "MOVS R0, #4 \n"
         "MOV R1, LR \n"
         "TST R0, R1 \n"
         "BEQ _MSP \n"
         "MRS R0, PSP \n"
         "B HardFault_HandlerCtt \n"
         "_MSP: \n"
         "MRS R0, MSP \n"
         "B HardFault_HandlerCtt \n"
         ".syntax divided\n") ;
}


#ifdef __cplusplus
} // extern "C"
#endif
