
// PICK #define ONE TO TEST:: USE WITH > #define INTV_TIME 1000
//#define Nmicros it_micros // Simple flag var usage version
//#define Nmicros FBit_micros // Cortex __LDREXW&__STREXW key on var
//#define INTV_TIME 1000 // This should be 1000 to simulate millis systick_isr

#define Nmicros FFit_micros // FAST FAKE :: USE WITH > #define INTV_TIME 100
#define INTV_TIME 100 // This should be 100 to run FAKE test with 10,000 _isr's/sec

#define LOOP_CNT  100000000 // 100000000 > count of contiguous us's require each loop()
volatile uint32_t it_millis_count = 0;
volatile uint32_t it_cycle_count = 0;
volatile uint32_t it_safe_read = 0;

// Uses independent flag var chared with _isr that can have known value set
uint32_t it_micros(void)  // Replaces micros() >> 100K micros. Took cycles# 1696302
{
  uint32_t ccdelta, usec, smc, scc;
  do {
    it_safe_read = 1;
    smc = it_millis_count;
    scc = it_cycle_count;
  } while ( 1 != it_safe_read ); // repeat if _isr
  ccdelta = ARM_DWT_CYCCNT - scc;
  usec = 1000 * smc + (ccdelta / (F_CPU_ACTUAL / 1000000));
  return usec;
}

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
// Run intervaltimer at 100us, code is 4 times slower, but accepts _isr 10 times faster
//
uint32_t FFcnt = 0; // allow resetting between loops - test will break on WRAP
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
  return FFcnt; // usec; // Return new FFcnt w/_isr before or during Read 'A'
}

IntervalTimer ITtest;
void it_systick_isr() { // REPLACES >> extern "C" void systick_isr(void)
  // ODDly it works without 'it_safe_read' changing in the _isr ( in this test )
  // comment it and see - but remove the __LDREXW instructions and it fails
  it_safe_read = 0; // This is the _isr change trigger variable
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

uint32_t Lcnt = 0;
void loop() {
  uint32_t rrN = 0, ii = 0, usn, nn;
  int stop = 0;
  Lcnt++;
  if ( FFcnt >  (0xffffffff - (LOOP_CNT + 1000)) ) FFcnt = 0; // for FFit_micros
  do {
    nn = 0;
    usn = Nmicros();
    do {
      rrN = Nmicros();
      if ( (usn + 1) == rrN )
        nn++;
      else if ( (usn + 2) == rrN ) // in case an extra us tick is missed ?
        nn++;
      else if ( rrN != usn ) // catch any out of order return
        stop = 666;
    } while ( 0 == nn && !stop );
    ii++;
    if ( LOOP_CNT == ii )
      stop = 1; // test pass complete
    if ( 1 != nn ) break;
    if ( !(ii % 100000) )   digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN ) );
  } while ( !stop );

  if ( nn != 1 && !stop )
    print2l( "Bad New us Val: nn=", nn );

  print2( "\tlast test New usn=", usn );
  print2l( "\tlast New us: rrN=", rrN );
  print2( "\t\tStop Val: ", stop );
  print2l( "\tNested Loops: ", ii );
  if ( rrN != (1 + usn)  || stop >= 10 )  print2l( "ERROR ERROR rrN != usn++  rrN:", rrN );
  print2( "\tcurrent Millis: ", millis() );
  print2( "\tINTV_TIME: ", INTV_TIME );
  print2l( "\tTotal Loops: ", Lcnt );
  delay( 1000 );
}
