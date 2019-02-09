//#define DEBUG_OFF // defining this hacks out all use and code for debug library.

#ifdef DEBUG_OFF
#define DebState( a ) 0
#define DebText( a )
#define deb_tt( a, b ) // DebSet( a, b, __LINE__, __func__ );
#define haltif_tt( a )
#define assert_tt( a ) // if (!a) { assert_3(__FILE__, __LINE__,  #a, __func__); }
#define qBlink() (GPIOC_PTOR = 32)  // Pin13 on T3.x & LC // requires: pinMode(LED_BUILTIN, OUTPUT);

/*
  #define DebSet( a,b,c,d )

  #define DebText( a )
  #define DebText2( a, b )

  #define debug_fault( a )

  #define assert_3( a,b,c,d )
  #define DebugWait( a )

*/
#else
#include "debug_tt.h"

// For haltif() entering 'd' calls this optional code
// It can call functions or dump globals.
void Debug_Dump(void)
{
  where_tt( );
  DebSer.print(" User Custom Debug Dump. Micros==");
  DebSer.println(micros());
  // Put code here to call after Fault or HaltIf() - or call it in sketch
  return;
}
#endif

#define BUTTON_ISR 3

void setup() {
  Serial1.begin( 115200 );
  Serial1.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  Serial.begin(115200);
  pinMode( BUTTON_ISR, INPUT_PULLUP );
  while (!Serial && millis() < 5000 );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  if ( !DebState(LED_BUILTIN) ) { // specify Blink Pin, pin is set to OUTPUT
    Serial.print("\n No Faults detected.");
    Serial.println("\tSHOULD END WITH >> Setup DONE! All Good?");
  }
  foo();
  where_tt( );
  addr_tt( setup );
  int ii;
  addr_tt( &ii );
  DebugTest2();
}

void foo( void ) {
  where_tt( );

  // test
  addr_tt( foo );
  int ii;
  addr_tt( &ii );
  int jj;
  addr_tt( &jj );


  // Code Here
}


FASTRUN void DebugTest2() {
  DebText( __func__ ); // show function entry
  deb_tt( 2, micros() );
  return;
  float x = log (0);
  haltif_tt( isfinite( x ) ); // Halt if float not NAN or INF
  deb_tt( 3, 0x101101 );
  Serial.printf("INF >> %f\n", x); // Hard Fault with printf()
  deb_tt( 6, millis() );
  x = 0.0 / 0.0;
  deb_tt( 100, 0x111011 );
  assert_tt( !isinf( x ) ); // Testing here - passes isinf()
  Serial.printf("NAN >> %f\n", x); // Hard Fault with printf()
  printf("NAN >> %f\n", x); // Hard Fault with printf()

  deb_tt( 9, __LINE__ );
  // IMXRT_BUGBUG   if ( UART2_C2 ) deb_tt( 9, __LINE__ );  // UART2 not initialized, accessing this register causes a Hard Fault
}

void loop() {
  static uint16_t ii = 0;
  deb_tt( ii, micros() );
  haltif_tt( digitalReadFast( BUTTON_ISR ) ); // Stop on button press
  ii++;
  ii &= 7;
  delay(750);
  qBlink();
  // Serial1.println("\n NOW FAULT ...");
  // GPT1_CNT = 5; // FAULT
  assert_tt( 0 );
}
