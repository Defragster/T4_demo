// ARM Synchronization Primitives :: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dht0008a/ch01s02s01.html
// https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=196050&viewfull=1#post196050
// https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=196115&viewfull=1#post196115
// https://github.com/PaulStoffregen/cores/pull/330

#define Nmicros micros // Cortex __LDREXW&__STREXW key on var
#define LOOP_CNT  25000000 // 25M is 25 secs to check contiguous us's each loop()
//#define LOOP_CNT  10000000 // 10M is 10 secs to check contiguous us's each loop()
//#define LOOP_CNT  1000000 // 1M is 1 sec to check contiguous us's each loop()

//#define CHANGE_SPEED // define to cycle the CPU speed
#define MAX_SPEED 800000000
#define MIN_SPEED 100000000
uint32_t ArmSpeed = 600000000;

// FrankB Demo :: https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=195979&viewfull=1#post195979

#define print2( a, b ) ({ Serial.print( a ); Serial.print( b ); })
#define print2l( a, b ) ({ Serial.print( a ); Serial.println( b ); })
extern "C" uint32_t set_arm_clock(uint32_t frequency);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, HIGH );
  while ( !Serial );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  set_arm_clock( ArmSpeed );
  delayMicroseconds( 100 );
  uint32_t dtime = ARM_DWT_CYCCNT;
  for ( uint32_t ii = 0; ii < 10000; ii++) {
    Nmicros();       Nmicros();       Nmicros();       Nmicros();       Nmicros();
    Nmicros();       Nmicros();       Nmicros();       Nmicros();       Nmicros();
  }
  dtime = ARM_DWT_CYCCNT - dtime;
  print2l("\t100K micros. Took cycles# ", dtime);
}

uint32_t Lcnt = 0, ErrFnd = 0;
void loop() {
  uint32_t rrN = 0, ii = 0, usn, nn, loopTime, dd;
  int stop = 0;
  Lcnt++;
#ifdef CHANGE_SPEED
  if ( !(Lcnt % 2) ) {      // Change Clock Speed
    ArmSpeed -= 100000000;
    if ( ArmSpeed < MIN_SPEED ) ArmSpeed = MAX_SPEED;
    set_arm_clock( ArmSpeed );
    if ( F_CPU_ACTUAL < MIN_SPEED ) ArmSpeed = MAX_SPEED;
    set_arm_clock( ArmSpeed );
  }
#endif
  print2("\t>>> 1ms systick :micros() >>> Clock Speed is:", F_CPU_ACTUAL);
  Serial.printf( "    °C=%2.2f\n" , tempmonGetTemp() );
  loopTime = millis();
  do {
    nn = 0;
    usn = Nmicros(); // should be usn++ - time of test logic can causes missed 1us ?
    do {
      rrN = Nmicros();
      dd = rrN - usn;
      if ( dd == 0 ) ;
      else if ( dd == 1 ) nn++;
#ifdef CHANGE_SPEED
      else if ( dd <= 2 ) // extra us ticks missed slower speeds
        nn++;
#else
      else if ( dd <= 2 && 1 == Lcnt ) // extra us tick may miss loop#1 startup @600 MHz
        nn++;
#endif
      else if ( dd >= (0 - 3) ) // catch any out of order return
        stop = 777;
      else // if ( 0 != dd ) // catch any out of order return
        stop = 666;
    } while ( 0 == nn && !stop );
    ii++;
    if ( LOOP_CNT == ii ) stop = 1; // test pass complete
    if ( 1 != nn ) break;
    if ( !(ii % 100000) )   digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN ) );
  } while ( !stop );

  loopTime = millis() - loopTime;
  if ( nn != 1 && !stop ) print2l( "Bad New us Val: nn=", nn );
  print2( "\tlast test New usn=", usn );
  print2l( "\tlast New us: rrN=", rrN );
  print2( "\t\tStop Val: ", stop );
  print2l( "\tNested Loops: ", ii );
  if ( rrN != (1 + usn)  || stop >= 10 )  {
    print2l( "ERROR ERROR rrN != usn++  rrN:", rrN );
    ErrFnd++;
  }
  if ( 4294967 > millis() && millis() != micros()/1000 )   print2l( "us!=ms", micros() );
  print2( "\tcurrent Millis: ", millis() );
  print2( "\tTotal Loops: ", Lcnt );
  print2l( "\tLoop time in ms=", loopTime );
  digitalWrite( LED_BUILTIN, HIGH );
  if ( 0 != ErrFnd )  {
    print2l( "\t# ERRORS Found:", ErrFnd );
    digitalWrite( LED_BUILTIN, LOW );
  }
  delay( 1000 );
}
