//#define DEBUG_OFF
#include "debug_t3.h"

#ifndef __MKL26Z64__ // T_LC Does not have this hardware
#include "kinetis_flexcan.h"
#define FLEXCANb_IMASK1(b)                (*(vuint32_t*)(b+0x28))
#define FLEXCANb_MB_MASK(b, n)            (*(vuint32_t*)(b+0x880+(n*4)))
#endif

/* HardFaults demonstrates some found way to cause a processor Fault
    Normally these just HALT the processor and STOP all USB or other output
    Using the above included library maps the fault response to code that
    provides displayable feedback as demonstrated within.
    Some Faults are recoverable and continue, others repeat forever
    Those that repeat stop all user output and function, this library
    Keeps that output flowing where possible and FAST blinks the LED
    >> Allows AUTO programming of the Teensy without Button as the USB survives
*/

void setup() {
  uint32_t read;
  Serial.begin(115200);
  while (!Serial && millis() < 5000 );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  if ( !DebState(LED_BUILTIN) ) { // specify Blink Pin
    Serial.println("\n Not FAULTED.");
  }

  deb_t3( 2, millis() );
  deb_t3( 401, 0 ); // Show collected debug info

#ifndef __MKL26Z64__ // T_LC Does not have this hardware
  deb_t3( 5, millis() );
  Serial.println(FLEXCANb_MB_MASK(FLEXCAN0_BASE, 0)); // will lock up because we didnt enter freeze mode from MCR register

  deb_t3( 5, millis() );
  Serial.println(FLEXCANb_IMASK1(FLEXCAN0_BASE));
#endif

  deb_t3( 2, millis() );
  float x = log (0);
  deb_t3( 6, 0xBAD1 );
//  printf("%f\n", x); // Hard Fault

  deb_t3( 8, micros() );
  x = 0.0 / 0.0;
  deb_t3( 7, 0x101010 );
//  printf("%f\n", x);

  uint8_t temp;
  deb_t3( 4, 222 );
  temp = UART2_C2;
  digitalWrite(LED_BUILTIN, 0);

  debug_fault( 0 );
  deb_t3( 401, 0 ); // Show collected debug info
  deb_t3( 9, 333 );
  TestFault();
  uint32_t *test = (uint32_t *)0x1FFFFFFE;
  deb_t3( 9, 444 );
  read = *test;
  deb_t3( 2, 1024 );
  *test += 1024;
  test = (uint32_t *)0x1FFFFFFF;
  deb_t3( 9, __LINE__ );
  ( 2, 33333 );
  *test += 33333;
  test = (uint32_t *)0x0;
  deb_t3( 2, 666 );
  test += 666;

  if ( DebState(NO_BLINK) ) {
    Serial.println("\n FAULTED!");
    qBlink();
    assert_t3( !DebState(NO_BLINK) ); // ASSERT if Fault was triggered
  }
}

void loop() {
  delay(750);
  qBlink();
}

// This runs loops to cross MCU memory boundary
// LC, 3.2, 3.5, 3.6 handle it differently
void TestFault() {
  uint32_t *foo2;
  uint16_t *foo1;
  uint8_t *foo;

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
      deb_t3( 8, foo[0] );
      delay(20);
      Serial.println( "-----@8" );
      deb_t3( 3, micros() );
      foo++;
    }
    for ( ii = 0; ii < 10; ii++ ) {
      Serial.print( (uint32_t)foo1, HEX );
      Serial.print( " :: " );
      deb_t3( 6, ii );
      delay(20);
      Serial.print( foo1[0], HEX );
      deb_t3( 0, foo1[0] );
      if ( zz > 0) foo1[0] = foo1[0] + 1;
      delay(20);
      deb_t3( 7, zz );
      Serial.println( "-----@16" );
      foo1 = (uint16_t *)(1 + (char *)foo1);
    }
  }
}

