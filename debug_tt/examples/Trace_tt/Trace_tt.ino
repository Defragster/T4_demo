//#define DEBUG_OFF // defining this hacks out all use and code for debug library.

#define SERIAL_tt Serial1 // Send debug_tt output here. Must have SERIAL_tt.begin( ## )
#define BUTTON_ISR 3

#ifdef DEBUG_OFF
//#define debState( a ) 0
#define debText_tt( a )
#define deb_tt( a, b )
#define haltif_tt( a )
#define assert_tt( a )
#define debText_tt( a )
#define debText_tt2( a, b )
#define debSet( a,b,c,d )
#define debug_fault( a )
#define assert_3( a,b,c,d )
#define debugWait( a )
#else
#include "debug_tt.h"

// For haltif() entering 'd' calls this optional code
// It can call functions or dump globals.
void Debug_Dump(void)
{
  where_tt( );
  SERIAL_tt.print(" User Custom Debug Dump. Micros==");
  SERIAL_tt.println(micros());
  // Put code here to call after Fault or HaltIf() - or call it in sketch
  debTraceShow_tt( -3, "CycCnt %u", "\tline %u", "\tfunc %s\n" );
  debTraceShow_tt( 0, "CycCnt %u", "\tline %u", "\tfunc %s\n" );
  return;
}
#endif

void yield() {}
void setup() {
  SERIAL_tt.begin( 1843200 );
  Serial4.begin( 1843200 );
  debTraceShow_tt( -1, "", "", "" );
  SERIAL_tt.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  SERIAL_tt.println("\n********\n T4 connected Serial1 *******\n");
  Serial4.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  Serial4.println("\n********\n T4 connected Serial4 *******\n");
  Serial.begin(115200);
  while (!Serial && millis() < 5000 );
  debTraceShow_tt( -2, "", "", "" );
  pinMode( BUTTON_ISR, INPUT_PULLUP );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  debBegin_tt( &SERIAL_tt, LED_BUILTIN, 12);

  Serial.println("\tSHOULD END WITH >> Setup DONE! All Good?");
  foo();
  where_tt( );
  addr_tt( setup );
  int ii;
  addr_tt( &ii );
  DebugTest2();
  debTrace_tt( ARM_DWT_CYCCNT, __LINE__, "You are here" );
  debText_tt( __func__ ); // show function entry
  for ( ii = 0; ii < 5; ii++ )
    debTrace_tt( ARM_DWT_CYCCNT, __LINE__, __func__ );
  debText_tt( __func__ ); // show function entry
  assert_tt( 0 );
  debTraceShow_tt( -3, "CycCnt %u", "\tline %u", "\tfunc %s\n" );
}

void foo( void ) {
  debTrace_tt( ARM_DWT_CYCCNT, __LINE__, __func__ );
  where_tt( );
  debTrace_tt( ARM_DWT_CYCCNT, __LINE__, __func__ );
  debTrace_tt( ARM_DWT_CYCCNT, __LINE__, __func__ );
  addr_tt( foo );
  debTrace_tt( ARM_DWT_CYCCNT, __LINE__, __func__ );
  int ii;
  addr_tt( &ii );
  debTrace_tt( ARM_DWT_CYCCNT, __LINE__, __func__ );
  int jj;
  addr_tt( &jj );
  // Code Here
}


void DebugTest2() {
  debText_tt( __func__ ); // show function entry
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
}

void loop() {
  static uint16_t ii = 0;
  deb_tt( ii, micros() );
  haltif_tt( digitalReadFast( BUTTON_ISR ) ); // Stop on button press
  ii++;
  ii &= 7;
  delay(2000);
  qBlink();
  assert_tt( 0 );
}
