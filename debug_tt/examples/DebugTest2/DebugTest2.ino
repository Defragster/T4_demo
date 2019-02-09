//#define DEBUG_OFF // defining this only shows Hard Faults, no other debug output
#include "debug_t3.h"

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000 );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  if ( !DebState(LED_BUILTIN) ) { // specify Blink Pin, pin is set to OUTPUT
    Serial.println("\n No Faults detected.");
  }
  DebugTest2();
  Serial.println("\n Setup DONE! All Good?");
}

void DebugTest2() {
  DebText( __func__ ); // show function entry
  deb_t3( 2, micros() );
  float x = log (0);
  haltif_t3( isfinite( x ) ); // Halt if float not NAN or INF
  deb_t3( 3, 0x101101 );
  Serial.printf("INF >> %f\n", x); // Hard Fault with printf()
  deb_t3( 6, millis() );
  x = 0.0 / 0.0;
  deb_t3( 100, 0x111011 );
  assert_t3( !isinf( x ) ); // Testing here - passes isinf()
  Serial.printf("NAN >> %f\n", x); // Hard Fault with printf()

  uint8_t temp; 
  deb_t3( 9, __LINE__ );
  temp = UART2_C2; // UART2 not initialized, accessing this register causes a Hard Fault
}

void loop() {
  delay(750);
  qBlink();
}

// For haltif() entering 'd' calls this optional code
// It can call functions or dump globals.
void Debug_Dump(void)
{
  DebSer.print(" User Custom Debug Dump. Micros==");
  DebSer.println(micros());
  return;
}
