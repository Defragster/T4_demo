// ARM Synchronization Primitives :: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dht0008a/ch01s02s01.html
// https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=196050&viewfull=1#post196050

#define Nmicros micros // Cortex __LDREXW&__STREXW key on var
#define LOOP_CNT  10000000 // 10M is 10 secs to check contiguous us's each loop()
//#define LOOP_CNT  1000000 // 1M is 1 sec to check contiguous us's each loop()

#define CHANGE_SPEED // define to cycle the CPU speed
uint32_t ArmSpeed = 600000000;
volatile uint32_t it_millis_count = 0;
volatile uint32_t it_cycle_count = 0;
volatile uint32_t it_safe_read = 0; // ref var for __LDREXW/__STREXW

// FrankB Demo :: https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=195979&viewfull=1#post195979

#define print2( a, b ) ({ Serial.print( a ); Serial.print( b ); })
#define print2l( a, b ) ({ Serial.print( a ); Serial.println( b ); })

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, HIGH );
  while ( !Serial );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  uint32_t dtime = ARM_DWT_CYCCNT;
  for ( uint32_t ii = 0; ii < 10000; ii++) {
    Nmicros();       Nmicros();       Nmicros();       Nmicros();       Nmicros();
    Nmicros();       Nmicros();       Nmicros();       Nmicros();       Nmicros();
  }
  dtime = ARM_DWT_CYCCNT - dtime;
  print2l("\t100K micros. Took cycles# ", dtime);
}

extern "C" uint32_t set_arm_clock(uint32_t frequency);
uint32_t Lcnt = 0, ErrFnd = 0;
void loop() {
  uint32_t rrN = 0, ii = 0, usn, nn, loopTime;
  int stop = 0;
  Lcnt++;
#ifdef CHANGE_SPEED
  if ( !(Lcnt % 3) ) {      // Change Clock Speed
    ArmSpeed -= 100000000;
    if ( ArmSpeed < 10000000 ) ArmSpeed = 700000000;
    set_arm_clock( ArmSpeed );
    if ( F_CPU_ACTUAL < 100000000 ) ArmSpeed = 700000000;
    set_arm_clock( ArmSpeed );
  }
#endif
  print2(">>> 1ms systick :micros() >>> Clock Speed is:", F_CPU_ACTUAL);
  Serial.printf( "    °C=%2.2f\n" , tempmonGetTemp() );
  loopTime = millis();
  do {
    nn = 0;
    usn = Nmicros(); // should be usn++ - but test logic can cause missed 1us
    do {
      rrN = Nmicros();
#ifdef CHANGE_SPEED
      if ( usn <= (rrN - 1 ) && ( usn <= rrN + 5 ) ) // extra us ticks missed slower speeds
        nn++;
#else
      if ( (usn + 1) == rrN )
        nn++;
      else if ( ( (usn + 2) == rrN ) && 1 == Lcnt ) // extra us tick may miss loop#1 startup @600 MHz
        nn++;
#endif
      else if ( rrN != usn ) // catch any out of order return
        stop = 666;
    } while ( 0 == nn && !stop );
    ii++;
    if ( LOOP_CNT == ii ) stop = 1; // test pass complete
    if ( 1 != nn ) break;
    if ( !(ii % 100000) )   digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN ) );
  } while ( !stop );

  if ( nn != 1 && !stop ) print2l( "Bad New us Val: nn=", nn );
  print2( "\tlast test New usn=", usn );
  print2l( "\tlast New us: rrN=", rrN );
  print2( "\t\tStop Val: ", stop );
  print2l( "\tNested Loops: ", ii );
  if ( rrN != (1 + usn)  || stop >= 10 )  {
    print2l( "ERROR ERROR rrN != usn++  rrN:", rrN );
    ErrFnd++;
  }
  print2( "\tcurrent Millis: ", millis() );
  print2( "\tTotal Loops: ", Lcnt );
  loopTime = millis() - loopTime;
  print2l( "\tLoop time in ms=", loopTime );
  if ( 0 != ErrFnd )  {
    print2l( "\t# ERRORS Found:", ErrFnd );
  }
  delay( 1000 );
}
