// ARM Synchronization Primitives :: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dht0008a/ch01s02s01.html
// https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=196050&viewfull=1#post196050

// Comment "#define FIRST" to run 10X speed test version versus ms timer
#define FIRST
#ifdef FIRST
#define Nmicros FBit_micros // Cortex __LDREXW&__STREXW key on var
#define INTV_TIME 1000 // This should be 1000 to simulate millis systick_isr
#define LOOP_CNT  10000000 // 10M is 10 secs to check contiguous us's each loop()
//#define LOOP_CNT  1000000 // 1M is 1 sec to check contiguous us's each loop()
#else
#define Nmicros FFit_micros // FAST FAKE :: USE WITH > #define INTV_TIME 100
#define INTV_TIME 100 // This should be 100 to run FAKE test with 10,000 _isr's/sec
#define LOOP_CNT  100000000 // 100M is 30 secs to count contiguous us's each loop()
#endif

#define CHANGE_SPEED // define to cycle the CPU speed
uint32_t ArmSpeed = 600000000;
volatile uint32_t it_millis_count = 0;
volatile uint32_t it_cycle_count = 0;
volatile uint32_t it_safe_read = 0; // ref var for __LDREXW/__STREXW

// FrankB Demo :: https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=195979&viewfull=1#post195979
#include "arm_math.h"
#include "core_cmInstr.h"
uint32_t FBit_micros(void)  // Replaces micros() >> 100K micros. Took cycles# 1795442
{
  uint32_t ccdelta, usec, smc, scc;
  do {
    __LDREXW(&it_safe_read);
    smc = it_millis_count;
    scc = it_cycle_count;
  } while ( __STREXW(1, &it_safe_read));
  ccdelta = ARM_DWT_CYCCNT - scc;
  usec = 1000 * smc + (ccdelta / (F_CPU_ACTUAL / 1000000));
  return usec;
}

// FAST FAKE return a new count if _isr was called based on data change
// Run intervaltimer at 100us, code is 4 times slower, but works with _isr 10 times faster
//
uint32_t FFcnt = 0, ErrFnd = 0;
uint32_t FFit_micros(void)  // Replaces micros() >> 100K micros. Took cycles# 8948674
{
  uint32_t ccdelta, usec, usec2, smc, scc, myCycCnt;
  static uint32_t lastU, isrB = 0;
  do { // Read 'A' of _isr data
    __LDREXW(&it_safe_read);
    smc = it_millis_count;
    scc = it_cycle_count;
  } while ( __STREXW(1, &it_safe_read));
  myCycCnt = ARM_DWT_CYCCNT;
  ccdelta = myCycCnt - scc;
  usec = 10000 * smc + (ccdelta / (F_CPU_ACTUAL / 10000000)); // this changed for FF
  do { // Read 'B' of _isr data
    __LDREXW(&it_safe_read);
    isrB++; // if >=2 _isr adjusted data and won't match Read 'A'
    smc = it_millis_count;
    scc = it_cycle_count;
  } while ( __STREXW(1, &it_safe_read));
  ccdelta = myCycCnt - scc;
  usec2 = 10000 * smc + (ccdelta / (F_CPU_ACTUAL / 10000000));
  if ( usec != usec2 && 1 == isrB )
    FFcnt = 0; // Two calc's are different. not OK unless _isr seen in Read 'B'
  else if ( usec != lastU ) {
    FFcnt++;
    lastU = usec;
  }
  return FFcnt; // Return new FFcnt w/_isr before or during Read 'A'
}

IntervalTimer ITtest;
void it_systick_isr() { // REPLACES >> extern "C" void systick_isr(void)
  it_cycle_count = ARM_DWT_CYCCNT;
  it_millis_count++;
  //  MillisTimer::runFromTimer(); // not for test use
}


#define print2( a, b ) ({ Serial.print( a ); Serial.print( b ); })
#define print2l( a, b ) ({ Serial.print( a ); Serial.println( b ); })

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, HIGH );
  while ( !Serial );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  uint32_t dtime = ARM_DWT_CYCCNT;
  ITtest.begin( it_systick_isr, INTV_TIME );
  for ( uint32_t ii = 0; ii < 10000; ii++) {
    Nmicros();       Nmicros();       Nmicros();       Nmicros();       Nmicros();
    Nmicros();       Nmicros();       Nmicros();       Nmicros();       Nmicros();
  }
  dtime = ARM_DWT_CYCCNT - dtime;
  print2l("\t100K micros. Took cycles# ", dtime);
}

extern "C" uint32_t set_arm_clock(uint32_t frequency);
uint32_t Lcnt = 0;
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
#ifdef FIRST
  print2(">>> 1ms timer :FBit_micros >>> Clock Speed is:", F_CPU_ACTUAL);
#else
  print2(">>> fast fake :FFit_micros >>> Clock Speed is:", F_CPU_ACTUAL);
#endif
  Serial.printf( "    °C=%2.2f\n" , tempmonGetTemp() );
  loopTime = millis();
  do {
    nn = 0;
    usn = Nmicros(); // should be usn++ - but test logic can cause missed 1us
    do {
      rrN = Nmicros();
#ifdef CHANGE_SPEED
      if ( usn <= (rrN-1 ) && ( usn <= rrN+5 ) ) // extra us ticks missed slower speeds
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
  print2( "\tINTV_TIME: ", INTV_TIME );
  print2( "\tTotal Loops: ", Lcnt );
  loopTime = millis() - loopTime;
  print2l( "\tLoop time in ms=", loopTime );
  if ( 0 != ErrFnd )  {
    print2l( "\t# ERRORS Found:", ErrFnd );
  }
  delay( 1000 );
}
