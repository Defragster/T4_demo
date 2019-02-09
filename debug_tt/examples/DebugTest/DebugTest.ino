//#define DEBUG_OFF
#include "debug_t3.h"

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000 );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  if ( !DebState(LED_BUILTIN) ) { // specify Blink Pin
    Serial.println("\n Not FAULTED.");
  }
  DebText( __FILE__ );
  DebText( __func__ );
  deb_t3( 9, __LINE__ );

  DebugTestFunc();
  Serial.println("\n Setup DONE! All Good?");
}

void DebugTestFunc() {
  uint32_t read;
  uint32_t *test = (uint32_t *)0x1FFFFFFE;
  DebText( __func__ );
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
  temp = UART2_C2;

  deb_t3( 9, __LINE__ );
  debug_fault( 0 ); // False Fault - Show debug info
  TestFault();
  qBlink();
  delay( 200);
  assert_t3( true ); // Assert this exppression is TRUE - no Assert
  deb_t3( 9, __LINE__ );
  //  assert_t3( false ); // Assert this exppression is TRUE - WILL Assert
  deb_t3( 9, __LINE__ );
  read = *test;
  deb_t3( 2, 1024 );
  *test += 1024; // recoverable memory Fault (on T_3.6)
  deb_t3( 5, 1024 );
  test = (uint32_t *)0x1FFFFFFF;
  deb_t3( 9, __LINE__ );
  deb_t3( 2, 33333 );
  *test += 33333; // recoverable memory Fault (on T_3.6)
  test = (uint32_t *)0x0;
  deb_t3( 2, 666 );
  deb_t3( 401, 666 );
  test += 666;
  deb_t3( 2, 22 );

  if ( DebState(LED_BUILTIN) ) { // detect if recoverable fault happened
    Serial.println("\n FAULTED!");
    qBlink();
    haltif_t3( !DebState(LED_BUILTIN) ); // ASSERT if Fault was triggered
  }
}

void loop() {
  delay(750);
  qBlink();
}

void TestFault() {
  uint32_t *foo2;
  uint16_t *foo1;
  uint8_t *foo;

  DebText( __FILE__ );
  uint32_t ii, zz;
  deb_t3( 0, 1 );
  for ( zz = 0; zz < 2; zz++ ) {
    deb_t3( 4, zz );
    foo2 = (uint32_t *)0x1FFFFFFA;
    foo1 = (uint16_t *)0x1FFFFFFA;
    foo = (uint8_t *)0x1FFFFFFA;
    for ( ii = 0; ii < 15; ii++ ) {
      deb_t3( 5, ii );
      Serial.print( (uint32_t)foo, HEX );
      Serial.print( " :: " );
      delay(20);
      foo[0] = ii;
      Serial.print( foo[0], HEX );
      deb_t3( 8, 5 );
      delay(20);
      Serial.println( "-----@8" );
      deb_t3( 8, 6 );
      foo++;
    }
    for ( ii = 0; ii < 10; ii++ ) {
      Serial.print( (uint32_t)foo1, HEX );
      Serial.print( " :: " );
      deb_t3( 6, ii );
      delay(20);
      Serial.print( foo1[0], HEX );
      deb_t3( 7, (uint32_t)foo1 );
      if ( zz > 0) foo1[0] = foo1[0] + 1;
      delay(20);
      deb_t3( 7, __LINE__ );
      Serial.println( "-----@16" );
      foo1 = (uint16_t *)(1 + (char *)foo1);
    }
  }
}

