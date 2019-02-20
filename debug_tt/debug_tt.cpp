#include <Arduino.h>  // types and class define
#include "debug_tt.h"

#ifndef NO_DEBUG_tt

// Macro #defines to map to USB for mk20dx128.c PJRC reg prints
#define ser_print dbprint
#define ser_print_hex32(a) dbprintH( a )

HardwareSerial *pdbser1 = NULL;
//#define dbprint( a ) { printf_tt( a ); }
//#define dbprintln( a ) { printf_tt( "%s\n", a ); }
#define dbprintf( a , b ) { printf_tt( a, b ); }
#define dbprintD( a ) { printf_tt( "%u", a ); }
#define dbprintDln( a ) { printf_tt( "%u\n", a ); }
#define dbprintH( a ) { printf_tt( "%x", a ); }
//#define dbprintln( a ) { if(pdbser1) {pdbser1->println( a );} }
//#define dbprFlush(  ) { if(pdbser1) {pdbser1->flush();} }
HardwareSerial *pdbser2 = (HardwareSerial *)&Serial; // BUGBUG - this is either not used or needs to be set by user - but helps debugging the debug library

#if defined(__IMXRT1052__)
#define faultPrintf( a, b ) { printf_tt( a, b ); }
#else
#define faultPrintf( a, b ) { printf_tt( a, b ); }
#endif

// debugprint.c :: LPUART3_BAUD = LPUART_BAUD_OSR(12) | LPUART_BAUD_SBR(1); // 1843200 baud // SERIAL_tt.begin( 1843200 );


/*
  >> DEBUG_BREAK versus _FAULT !!!!
  >> Change text presented and LED blink rate
  >> Use consitent Print Func? Map to clone of PJRC printf_debug - but send string to pdbser1-> unless faulted?

  Fast log calls save data and location, time stamp?
  incrment on each log call 0-63 ++&0x3f
  log_tt (vala, b, c) Store a, b, c and line# : can be timestamp millis/micros/CycCnt
  logShow_tt( x ) : print last x trace values from last value, show backwards to oldest, passing x==0 will clear the trace data

  >> Allow Assert/Halt disable of a break [up to 5 __LINE__&&__FILE__] from CMD prompt, clear on Zero debug info

  Make DLOG storage logical not *2+1 in Back[], add store of __func *sz for display

  All _tt Func/Macros should set a LastLine==__LINE__ for display at Fault

  Include in keywords the MAGIC number ofr Log Start/Stop/Clear and SHOW log ...

  refactor to debug_td for TeensyDebug - put into files:: Fault Entry, Log, Notes.txt, readme.me, keywords.txt
  > https://www.arduino.cc/en/Hacking/LibraryTutorial
  > https://www.arduino.cc/en/Reference/APIStyleGuide
*/

#define DTRACE_SIZE 1023
const uint32_t TraceMask = 1023;
volatile uint32_t TraceInfo[ ( DTRACE_SIZE ) ][5];
volatile int TraceIndex = 0;
volatile int TraceCnt = 0;
volatile bool TraceOn = true;
uint32_t lastL_tt = 0;
char * lastF_tt = 0;

// CALL TRACEshow from debug_tt break
// pass -1 == MAX to stop log
// pass -2 == MAX to start log

void debTrace_ttf( uint32_t aa, uint32_t bb, const char *cc, int __lineno, const char *__func) {
  if ( !TraceOn ) return;
  TraceCnt++;
  TraceInfo[ TraceIndex ][0] = aa;
  TraceInfo[ TraceIndex ][1] = bb;
  TraceInfo[ TraceIndex ][2] = (uint32_t)cc;
  TraceInfo[ TraceIndex ][3] = (uint32_t)__lineno;
  TraceInfo[ TraceIndex ][4] = (uint32_t)__func;
  ++TraceIndex;
  TraceIndex &= TraceMask;
}

// MAX is max # to print. -1 stops Trace recording, -2 enables, -3 says print all, 0 says to clear the Trace data
void debTraceShow_tt( int Max, const char *aa, const char *bb, const char *cc) {
  int ii, jj = 0, kk = TraceCnt;
  bool WasTraceOn = TraceOn;
  if ( 0 == Max ) {
    TraceCnt = 0;
    TraceIndex = 0;
    return;
  }
  else if ( -1 == Max ) {
    TraceOn = false;
    return;
  }
  else if ( -2 == Max ) {
    TraceOn = true;
    return;
  }
  else {
    TraceOn = false;
    if ( -3 == Max ) {
      Max = DTRACE_SIZE;
    }
    if ( TraceIndex > 0) {
      for ( ii = TraceIndex - 1; ii >= 0 && jj < kk && jj < Max; ii--) {
        jj++;
        printf_tt( "#%u: ", jj );
        //faultPrintf( "(ii=%u): ", ii );
        faultPrintf( aa, TraceInfo[ ii ][0] );
        faultPrintf( bb, TraceInfo[ ii ][1] );
        faultPrintf( cc, (char *)TraceInfo[ ii ][2] );
        faultPrintf( "\tin %s() ", (char *)TraceInfo[ ii ][4] );
//        faultPrintf( "\tin %s() ", (char *)TraceInfo[ ii ][4] );
// DEBUG FIFO loop #        faultPrintf( "[ii:%u]",  ii );
        faultPrintf( "L#:%u\n", TraceInfo[ ii ][3] );
        // delayMicroseconds( 30 );
      }
    }
    if ( kk > TraceIndex ) {
      for ( ii = DTRACE_SIZE - 1; ii >= TraceIndex && jj < kk && jj < Max; ii--) {
        jj++;
        faultPrintf( "#_%u: ", jj );
        //faultPrintf( "(ii=%u): ", ii );
        faultPrintf( aa, TraceInfo[ ii ][0] );
        faultPrintf( bb, TraceInfo[ ii ][1] );
        faultPrintf( cc, (char *)TraceInfo[ ii ][2] );
        faultPrintf( "\tin %s() ", (char *)TraceInfo[ ii ][4] );
// DEBUG FIFO loop #        faultPrintf( "[ii:%u]",  ii );
        faultPrintf( "L#:%u\n", TraceInfo[ ii ][3] );
        // delayMicroseconds( 30 );
      }
    }
    TraceOn = WasTraceOn;
  }
}


#define DLOG_SIZE 10
uint32_t DebInfo[ (DLOG_SIZE ) ] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t DebBack[ (DLOG_SIZE * 2) + 1 ] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint32_t DebCnt = 0;  // Running count of incoming calls to track order
const char *last_func;

#define DEBUG_ASSERT 666
#define DEBUG_HALTIF 911
#define DEBUG_SHOW  401
#define DEBUG_T4  7 // HardFault_HandlerC for T4
#define DEBUG_ISR  8 // _isr Triggered
#define MAX_FLT_ISR 8

#ifdef __cplusplus
extern "C" {
#endif

void unused_interrupt_vector(void);

void FlushPorts( void );

uint32_t FreeMem() { // for Teensy 3.0
  uint32_t stackTop;
  uint32_t heapTop;

  // current position of the stack.
  stackTop = (uint32_t) &stackTop;

  // current position of heap.
  void* hTop = malloc(1);
  heapTop = (uint32_t) hTop;
  free(hTop);

  // The difference is (approximately) the free, available ram.
  return stackTop - heapTop;
}

static int16_t _DoBlink = LED_BUILTIN; // default Teensy pin 13



/* FOR USE:: 1st Param is array index 0-9  , 2nd Param is uint32_t value.
    deb_tt( 0,1 );
    deb_tt( 9,var_foo );
   Use as appropriate to track location or data
  > update for Macro use deb_tt() that calls DebSet()
   Macro tracks the line number where called
*/


// TODO:
// New T4 as _tt
// Store Func name ALL *'s as well?
// Add IGNORE to prompts if Func&Line the same, clear on 'z' zero debug
// Seperate list of last 10 calls in order
// Start Debug_T4 rework to *Serial
// allow enable/disable of debug(pt's) to freeze data?
// Get T4 resetReason Code
// Register defs and Fault info
// {} Specify an _isr() break pin
// T4 to 800 MHz in startup faults/restarts.
// Get Reset {and bootloader} command
// {} On Fault show debug trace info
// Allow Dump and Code call on Fault, bootloader, reset
// deb_tt( NEXT_tt, millis() ); where NEXT_tt just loops with INC++ 63
// Put test calls in Startup code
// Start with NULL Stream* and don't print!  - Partially done
//++ On Halt/assert show info and allow return
// change _tt to _td - No :: tt is Teensy Trace
// Fault time Serialstuck: https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=196791&viewfull=1#post196791
// >> FIFO FILLING - FIFO goes empty without interrupts, need a way to poll it to process buffered data

//
// AKA: _t3 - Prior TODO's
// Debugging-strategies Forum :: https://forum.pjrc.com/threads/42728-Debugging-strategies
// {} ReWrite code to use PTR* to Serial? as given.
// Add tracing that toggles or set/clr a pin - and stores values?
// Add a wait for Serial in case FAULT triggers before it connects
// Compile size change montior for debug_tt overhead Flash 2852 B/RAM 112 B
// Add Verbose - print each DebSet() #'s' as passed - not just on a STOP
// {} Allow directed output Serial or Serial#
// {} Allow directed EACH DebSet() to output Serial or Serial#  > pass Stream *
// Allow request REBOOT/RESTART on FAULT - with stop count until cleared via EEPROM/Serail#.read
// Default SerOut = Serial and SerOut2=Serial, allow setting
// Test EEPROM record on fault - and report on RUN START
// Put data in an array to fault float - LOG(x)&&(/0.0), loop array and hit the fault
// Allow Disable HALT on assert_tt
// Save const char* to last __file__ through MACRO deb_ts to DebSet()
// export DebugBlink() for use in setup() etc as PinBlink( pin, time )
// Keep circBuffer of recent line # call order [versus current number list only]
// Keep circBuffer of recent Values call order
// Example faults in Functions usage
// DebText( ) :: send text to output
// DebText2( a , b ) :: send text to output for Text label and Value
// MCU RESET from Fault/Haltif
// Add Timers with memory and Print :: Time_td( #1 , 0) & stop==Time_td( #1 , "done test"), always return TimerLastVal
//   3 timers :: T_ms, T_us, T_cc
// Add way to use counters ( accumulators? ) for temp usage
// to leave NULL macros and include NO CODE bring a local copy of debug_td.h and use _OFF ??
// Why does printf LTO fail compile?
// On HaltIf allow stop on debug DebSet? calls? - must keep that FAST ???
// Add DebRead() to return stored value from stored DebInfo[]
// Add int pin to break on next call or stop with int_isr()
// Test overt out of range READ 2MB
// . . .   working
// T_LC no cycle counter for DebugBlink - adjust timing
// - T_LC - resolve with counted calls to debugBlink()
// Make a timer interrupt Sample of no print recording
// HaltIf sample on pin HIGH to show value dumping and Halt
// ... test/demo ifhalt( digitalread (pin) ) to stop execution and allow dumping values
// Added print of resetReason to init call of library
// Added Serail.read for :: bootloader, continue, restart, call 'weak' debugDump()
// Examples: Asserts and HardFaults [DebugTest]
// HaltIf:: versus Assert to allow pause on condition { [?disable?] , continue }
// Allow doBlink on pin# - not harcode #13 LED_BUILTIN :: DebState( Pin# {NO_BLINK} )
// Add a Timebegin and end?  Return as cycle counts
// void Debug_Dump(void) - weak for user dump of custom global values
//   > Allow HaltIf() to call a user function to dump or change data or save state to EEPROM or provide added UI for adjustment
// >>>>>   tested:
// Power on note of last shutdown reason?
// On Halt - allow ZERO of log data
// #define NO_DEBUG_tt that renders all the statements NULL for 'normal' compile, w/fault catch.
// Make Blink pause function with FlushPorts() assure AutoProgram
// User call to debug_fault( DEBUG_SHOW ) to just display the index call ist
//  >> or deb_tt( ii, # ) with index ii >= than DLOG_SIZE
//  >> ON deb_tt( ) with param 1 > DLOG_SIZE, don't dump regs!
// NaN test code works  :: assert_tt( !isnan( x ) );
// rename t3deb to deb_tt, like assert_tt debug_tt
// FYI:
// Toggle any pin? :: *portToggleRegister(LEDPIN) = 1;

char _Ftype[][4] = { "SHO", "nmi", "hrd", "mem", "bus", "use", "scv", "T_4", "isr" }; // Names for incoming fault reference

// These take over the weak fault handlers that are NULL in base TeensyDuino code
void nmi_isr(void)
{
  debug_fault( 1 );
}
void hard_fault_isr(void)
{
  debug_fault( 2 );
}
void memmanage_fault_isr(void)
{
  debug_fault( 3 );
}
void bus_fault_isr(void)
{
  debug_fault( 4 );
}
void usage_fault_isr(void)
{
  debug_fault( 5 );
}
void svcall_isr(void)
{
  debug_fault( 6 );
}


// TODO NEW FUN() :: DebState_tt to show trace/logged info. ??? returns # of faults passed ???
volatile int32_t inDF_tt = 0;

void debtt_isr() {
  if ( 0 != inDF_tt ) return;
  debug_fault( DEBUG_ISR ); // DUMP OUT debug_tt TRACE LOG
}

// Call with param of &Serial, NO_BLINK without blink or else Pin_#,
uint16_t debBegin_tt(  HardwareSerial *pserial, uint16_t DoBlink, uint32_t DoIsr ) {
  _DoBlink = DoBlink;
  if ( _DoBlink != NO_BLINK )   pinMode( _DoBlink, OUTPUT ); // Must enable output to blink
  if ( DoIsr != NO_ISR ) attachInterrupt( DoIsr, debtt_isr, CHANGE);
  int resetReasonHw;
#if defined(__IMXRT1052__)
  resetReasonHw = 0; // TODO T4 Restart Reason
#else
  resetReasonHw = RCM_SRS0;
  resetReasonHw |= (RCM_SRS1 << 8);
#endif

  if ( pserial ) {
    pdbser1 = pserial;
    resetReason( resetReasonHw );
    pdbser1->print( "F_CPU==");
#if defined(__IMXRT1052__)
    pdbser1->print( F_BUS_ACTUAL);
#else
    pdbser1->print( F_CPU);
    pdbser1->print( "   F_BUS==");
    pdbser1->println( F_BUS);
#endif
  }

  pdbser1->print(" FreeMem(); ");
  pdbser1->println(FreeMem());

  return DebBack[ DLOG_SIZE * 2 ];
}

extern "C" void debSet( uint16_t a, uint32_t b, uint16_t lineN, const char *__func ) {
  last_func = __func;
  if ( DLOG_SIZE <= a )
    debug_fault( DEBUG_SHOW ); // call fault code to dump registers
  else {
    DebInfo[ a ] = b;
    DebBack[ a + DLOG_SIZE ] = ++DebCnt;
    DebBack[ a ] = lineN;
  }

}

#ifndef __IMXRT1052__
#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);
#else
#define CPU_RESTART   asm("bkpt #251"); // run bootloader IMXRT
void _reboot_Teensyduino_(void) { asm("bkpt #251"); }
#endif

// handle diagnostic information given by assertion and abort program execution:
void assert_ttf(const char *__file, int __lineno, const char *__sexp, const char *__func) {
  // transmit diagnostic information through serial link.
  last_func = __func;
  DebugBlink( 0 );
#if 0
  int pp = 1000;
  while ( pp ) {
    FlushPorts();
    pp--;
  }
#endif
  printf_tt(" ___ ASSERT FAILED ___ FILE >> ");
  printf_tt("%s\n", __file);
  printf_tt(" ___ in function >> %s()", __func);
  printf_tt(" at LINE# >> %u", __lineno);
  printf_tt(" Expression >> (%s)\n", __sexp);
//BUGBUG print
  debug_fault( DEBUG_ASSERT ); // call fault code to dump registers
  // PAUSE program execution.
  printf_tt(" ___ ASSERT FAILED ___ 'y' to continue ...\n");
  printf_tt("\t 'd' to call Debug_Dump()\n");
  printf_tt("\t 'b' Teensy Bootloader\n");
  printf_tt("\t 'r' Restart Teensy\n");
  printf_tt("\t 'z' Zero Debug logs\n");
  DebugBlink( 0 );
  bool _wait = true;
  char foo;
  while (pdbser1->read() != -1) ; // Make sure queue is empty.
  while ( _wait ) {
//xxx    FlushPorts();
    DebugBlink( 125 );
    while ( pdbser1->available() ) {
      DebugBlink( 100 );
      foo = pdbser1->read();
      if ( 'y' == foo ) _wait = false;
      else if ( 'd' == foo ) Debug_Dump();
      else if ( 'b' == foo ) _reboot_Teensyduino_(); // goes to bootloader
      else if ( 'r' == foo ) { CPU_RESTART; } // restart CPU
      else if ( 'z' == foo ) 
      {
        for ( int ii = 0; ii < DLOG_SIZE; ii++ ) {
          DebCnt = 0;
          DebInfo[ ii ] = 0;
          DebBack[ DLOG_SIZE + ii ] = 0;
          DebBack[ ii ] = 0;
        }
      }
    }
    // while (pdbser1->read() != -1) ; // Make sure queue is empty.
  }
  printf_tt("end ASSERT\n");
}


// handle diagnostic information given by assertion and abort program execution:
void haltif_ttf(const char *__file, int __lineno, const char *__sexp, const char *__func) {
  // transmit diagnostic information through serial link.
  last_func = __func;
  pdbser1->print(" ___ HALT in ___ FILE >> ");
  pdbser1->println(__file);
  pdbser1->flush();
  pdbser1->print(" ___ in function >> ");
  pdbser1->print(__func);
  pdbser1->print("()  at LINE# >> ");
  pdbser1->println(__lineno, DEC);
  pdbser1->print(" Expression >> ");
  pdbser1->println(__sexp);
  pdbser1->flush();

  debug_fault( DEBUG_HALTIF ); // call fault code to dump registers
  // halt program execution.
  pdbser1->println(" ___ HALT Triggered ___ 'y' to continue ...");
  pdbser1->flush();
  pdbser1->println("\t 'd' to call Debug_Dump()");
  pdbser1->println("\t 'b' Teensy Bootloader");
  pdbser1->println("\t 'r' Restart Teensy");
  pdbser1->println("\t 'z' Zero Debug logs");
  pdbser1->flush();
  DebugBlink( 0 );
  bool _wait = true;
  char foo;
  while (pdbser1->read() != -1) ; // Make sure queue is empty.
  while ( _wait ) {
    FlushPorts();
    FlushPorts(); // avoid failure to reprogram
    DebugBlink( 200 );
    while ( pdbser1->available() ) {
      DebugBlink( 100 );
      foo = pdbser1->read();
      if ( 'y' == foo ) _wait = false;
      if ( 'd' == foo ) Debug_Dump();
      if ( 'b' == foo ) _reboot_Teensyduino_(); // goes to bootloader
      if ( 'r' == foo ) CPU_RESTART; // restart CPU
      if ( 'z' == foo ) {
        for ( int ii = 0; ii < DLOG_SIZE; ii++ ) {
          DebCnt = 0;
          DebInfo[ ii ] = 0;
          DebBack[ DLOG_SIZE + ii ] = 0;
          DebBack[ ii ] = 0;
        }
      }
    }
  }
  printf_tt("end Halt\n");
}

void debug_fault( int iFrom )
{
  inDF_tt = 1;
  if ( _DoBlink != NO_BLINK) digitalWrite( _DoBlink, !digitalRead(_DoBlink));
  static int cnt = 0;

  if ( DEBUG_ASSERT == iFrom ) {
    printf_tt(  "\n >>>> ASSERT  >>>> ASSERT FAILED   "  );
    DebBack[ DLOG_SIZE * 2 ] = iFrom;
    if ( cnt > 0 ) cnt--;
  }
  else if ( DEBUG_HALTIF == iFrom ) {
    printf_tt(  "\n >>>> HALT If true  >>>> program Paused   "  );
    DebBack[ DLOG_SIZE * 2 ] = iFrom;
    if ( cnt > 0 ) cnt--;
  }
  else if ( DEBUG_SHOW == iFrom ) {
    printf_tt(  "\n #### Debug trace Show "  );
  }
  else {
    FlushPorts();
    printf_tt(  "\n >>>> debug_fault   >>>> debug_fault   >>>> TYPE:"  );
    if ( iFrom <= MAX_FLT_ISR)
      printf_tt( "%s\n", _Ftype[ iFrom ] );
    else
      printf_tt( "%u", iFrom );
    DebBack[ DLOG_SIZE * 2 ] = iFrom;
  }
  printf_tt(  "debug_tt Info:"  );
  if ( 0 != lastL_tt && 0 != lastF_tt ) {
    printf_tt( "\t[Last debug_tt helper @ L#:%u  %s()", lastL_tt, lastF_tt );
  }
  printf_tt("\n");

  FlushPorts();
  for ( int ii = 0; ii < DLOG_SIZE; ii++ ) {
    if ( DebBack[ DLOG_SIZE + ii ] ) {
      printf_tt(  "%u => %u", ii, DebInfo[ ii ]  );
      printf_tt(  "\t%x", DebInfo[ ii ] );
      printf_tt(  "\t[L#%u", DebBack[ ii ] );
      printf_tt(  "_C#%u", DebBack[ DLOG_SIZE + ii ] );
      FlushPorts();
      if ( DebCnt ==  DebBack[ DLOG_SIZE + ii ]) {
        printf_tt(  " _<< last func:: %s()", last_func  );
      }
      printf_tt( "\n");
    }
    FlushPorts();
  }
  FlushPorts();
  if ( iFrom > MAX_FLT_ISR) {
    inDF_tt = 0;
    return;
  }
  if ( 0 != iFrom ) cnt++;

#ifndef __IMXRT1052__
  uint32_t addr; // from: ...\hardware\teensy\avr\cores\teensy3\mk20dx128.c
  FlushPorts();

  SIM_SCGC4 |= 0x00000400;
  UART0_BDH = 0;
  UART0_BDL = 26; // 115200 at 48 MHz
  UART0_C2 = UART_C2_TE;
  PORTB_PCR17 = PORT_PCR_MUX(3);
  printf_tt("\nfault: \n??: ");
  asm("ldr %0, [sp, #52]" : "=r" (addr) ::);
  printf_tt("%x\t\t??: ", addr);
  asm("ldr %0, [sp, #48]" : "=r" (addr) ::);
  printf_tt("%x\t\t??: ", addr);
  asm("ldr %0, [sp, #44]" : "=r" (addr) ::);
  printf_tt("%x\npsr: ", addr);
  asm("ldr %0, [sp, #40]" : "=r" (addr) ::);
  printf_tt("%x\t\tadr: ", addr);
  asm("ldr %0, [sp, #36]" : "=r" (addr) ::);
  printf_tt("%x\t\tlr: ", addr);
  asm("ldr %0, [sp, #32]" : "=r" (addr) ::);
  printf_tt("%x\nr12: ", addr);
  asm("ldr %0, [sp, #28]" : "=r" (addr) ::);
  printf_tt("%x\t\tr3: ", addr);
  asm("ldr %0, [sp, #24]" : "=r" (addr) ::);
  printf_tt("%x\nr2: ", addr);
  asm("ldr %0, [sp, #20]" : "=r" (addr) ::);
  printf_tt("%x\nr1: ", addr);
  asm("ldr %0, [sp, #16]" : "=r" (addr) ::);
  printf_tt("%x\nr0: ", addr);
  asm("ldr %0, [sp, #12]" : "=r" (addr) ::);
  printf_tt("%x\nr4: ", addr);
  asm("ldr %0, [sp, #8]" : "=r" (addr) ::);
  printf_tt("%x\nlr: ", addr);
  asm("ldr %0, [sp, #4]" : "=r" (addr) ::);
  printf_tt("%x\n", addr);
  asm("ldr %0, [sp, #0]" : "=r" (addr) ::);
  printf_tt( "---\n" );
#ifndef __MKL26Z64__ // T_LC exclude 
  if ( iFrom > 0 && iFrom <= MAX_FLT_ISR ) {
    uint32_t *test = (uint32_t *)0xe000ed28;
    char _Freg[][5] = { "CFR:", "HFR:", "DFR:", "MMA:", "BFA:", "ASR" }; // pg 386 YIU ARM

    uint32_t tii = 0;
    FlushPorts();
    for ( tii = 0; tii < 6; tii++ ) {
      FlushPorts();
      printf_tt( "%s :: %x\n", _Freg[tii], test[tii] );
      FlushPorts();
    }
  }
#endif
#endif
  int pp = 1000;
  DebugBlink( 0 );
  while ( pp ) {
    FlushPorts();
    pp--;
  }

  if ( _DoBlink != NO_BLINK) digitalWrite( _DoBlink, !digitalRead(_DoBlink));
  if ( cnt >= 1 ) {
    if ( iFrom <= MAX_FLT_ISR ) {
      printf_tt(  "\n >>>> debug_fault   >>>> TYPE:%s", _Ftype[ iFrom ] );
    }
    printf_tt( "\n--- Faulted >>>>  'y' to continue ...\n");
    printf_tt("\t 'd' to call Debug_Dump()\n");
    printf_tt("\t 'b' Teensy Bootloader\n");
    printf_tt("\t 'r' Restart Teensy\n");
    printf_tt("\t 'z' Zero Debug logs\n");
    bool _wait = true;
    char foo;
    while (pdbser1->read() != -1) ; // Make sure queue is empty.
    while ( _wait ) {
      FlushPorts();
      DebugBlink( 125 );
      while ( pdbser1->available() ) {
        DebugBlink( 100 );
        foo = pdbser1->read();
        if ( 'y' == foo ) _wait = false;
        else if ( 'd' == foo ) Debug_Dump();
        else if ( 'b' == foo ) _reboot_Teensyduino_(); // goes to bootloader
        else if ( 'r' == foo ) { CPU_RESTART; }  // restart CPU
        else if ( 'z' == foo ) {
          for ( int ii = 0; ii < DLOG_SIZE; ii++ ) {
            DebCnt = 0;
            DebInfo[ ii ] = 0;
            DebBack[ DLOG_SIZE + ii ] = 0;
            DebBack[ ii ] = 0;
          }
        }
      }
    }
  }
  printf_tt("end Fault\n");
  inDF_tt = 0;
  return;
}

// BUGBUG - PASS PARAM #1 - indicate when FAULT or just standard output loop
void FlushPorts( void ) {
  DebugBlink( 100 );
  return; // BUGBUG
  //if (_VectorsRam[IRQ_LPUART6 +16] != &unused_interrupt_vector) (*_VectorsRam[IRQ_LPUART6 +16])();

#ifndef __IMXRT1052__
  if (SIM_SCGC4 & SIM_SCGC4_USBOTG) usb_isr();
  if (SIM_SCGC4 & SIM_SCGC4_UART0) uart0_status_isr();
  if (SIM_SCGC4 & SIM_SCGC4_UART1) uart1_status_isr();
  if (SIM_SCGC4 & SIM_SCGC4_UART2) uart2_status_isr();
#elif defined(__MK64FX512__)
  // TODO - TEST T_ttf.5/3.6 UART3 and UART4 and Serial6
  if (SIM_SCGC4 & SIM_SCGC4_UART3) uart3_status_isr();
  if (SIM_SCGC1 & SIM_SCGC1_UART4) uart4_status_isr();
  if (SIM_SCGC1 & SIM_SCGC1_UART5) uart5_status_isr();
#elif defined(__MK66FX1M0__)
  // TODO - TEST T_ttf.5/3.6 UART3 and UART4 and Serial6
  if (SIM_SCGC4 & SIM_SCGC4_UART3) uart3_status_isr();
  if (SIM_SCGC1 & SIM_SCGC1_UART4) uart4_status_isr();
  if (SIM_SCGC2 & SIM_SCGC2_LPUART0) lpuart0_status_isr();
#endif
}

#ifdef __MKL26Z64__ // T_LC Does not have CycleCounter
void DebugBlink( uint32_t wait ) {
#define MilliFCPU (F_CPU / 450000L) // F_CPU cycles per ms
  static uint32_t LastCnt = 0;
  if ( (0 == wait) || wait < ( ( LastCnt ) / MilliFCPU ) ) {
    LastCnt = 0;
    if ( _DoBlink != NO_BLINK) digitalWrite( _DoBlink, !digitalRead(_DoBlink));
  }
  LastCnt++;
}
#else
// TODO - rename DebugBlink - incorporate selective pin toggle and delay
// return TRUE when prior call was 'wait' milliseconds prior
void DebugBlink( uint32_t wait ) {
#define MilliFCPU (F_CPU / 1000L) // F_CPU cycles per ms
  static uint32_t LastCnt = 0;
  ARM_DEMCR |= ARM_DEMCR_TRCENA; // Assure Cycle Counter active
  ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
  if ( (0 == wait) || wait < ( ( ARM_DWT_CYCCNT - LastCnt ) / MilliFCPU ) ) {
    LastCnt = ARM_DWT_CYCCNT;
    if ( _DoBlink != NO_BLINK) digitalWrite( _DoBlink, !digitalRead(_DoBlink));
  }
}
#endif

void dPromptUser() {
  bool _wait = true;
  char foo;
  while ( _wait ) {
    FlushPorts();
    FlushPorts(); // avoid failure to reprogram
    DebugBlink( 200 );
    while (pdbser1->read() != -1) ; // Make sure queue is empty.
    while ( pdbser1->available() ) {
      DebugBlink( 100 );
      foo = pdbser1->read();
      if ( 'y' == foo ) _wait = false;
      if ( 'd' == foo ) Debug_Dump();
      if ( 'b' == foo ) _reboot_Teensyduino_(); // goes to bootloader
      if ( 'r' == foo ) CPU_RESTART; // restart CPU
      if ( 'z' == foo ) {
        for ( int ii = 0; ii < DLOG_SIZE; ii++ ) {
          DebCnt = 0;
          DebInfo[ ii ] = 0;
          DebBack[ DLOG_SIZE + ii ] = 0;
          DebBack[ ii ] = 0;
        }
      }
    }
  }
}


void resetReason( int resetReasonHw ) {
  uint16_t mask = 1;
  pdbser1->print(">>> Reason for 'reset': ");
  pdbser1->print(resetReasonHw, HEX);
  do {
    switch (mask & resetReasonHw) {
    //RCM_SRS0
    case 0x0001: pdbser1->print(F(" wakeup")); break;
    case 0x0002: pdbser1->print(F(" LowVoltage"));  break;
    case 0x0004: pdbser1->print(F(" LossOfClock")); break;
    case 0x0008: pdbser1->print(F(" LossOfLock")); break;
    //case 0x0010 reserved
    case 0x0020: pdbser1->print(F(" wdog")); break;
    case 0x0040: pdbser1->print(F(" ExtResetPin")); break;
    case 0x0080: pdbser1->print(F(" PwrOn")); break;

    //RCM_SRS1
    case 0x0100: pdbser1->print(F(" JTAG")); break;
    case 0x0200: pdbser1->print(F(" CoreLockup")); break;
    case 0x0400: pdbser1->print(F(" SoftWare")); break;
    case 0x0800: pdbser1->print(F(" MDM_AP")); break;

    case 0x1000: pdbser1->print(F(" EZPT")); break;
    case 0x2000: pdbser1->print(F(" SACKERR")); break;
      //default:  break;
    }
  } while (mask <<= 1);
  pdbser1->println(" :: done Reason");
}



#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef NO_DEBUG_tt



// Teensy_DELAY without cycle counter?  :: USE micros()?
/*
  uint32_t micros(void)
  {
  uint32_t count, current, istatus;

  __disable_irq();
  current = SYST_CVR;
  count = systick_millis_count;
  istatus = SCB_ICSR; // bit 26 indicates if systick exception pending
  __enable_irq();
   //systick_current = current;
   //systick_count = count;
   //systick_istatus = istatus & SCB_ICSR_PENDSTSET ? 1 : 0;
  if ((istatus & SCB_ICSR_PENDSTSET) && current > 50) count++;
  current = ((F_CPU / 1000) - 1) - current;
  #if defined(KINETISL) && F_CPU == 48000000
  return count * 1000 + ((current * (uint32_t)87381) >> 22);
  #elif defined(KINETISL) && F_CPU == 24000000
  return count * 1000 + ((current * (uint32_t)174763) >> 22);
  #endif
  return count * 1000 + current / (F_CPU / 1000000);
  }

  void delay(uint32_t ms)
  {
  uint32_t start = micros();

  if (ms > 0) {
    while (1) {
      while ((micros() - start) >= 1000) {
        ms--;
        if (ms == 0) return;
        start += 1000;
      }
      yield();
    }
  }
  }
*/


// #include "imxrt.h"
// #include "wiring.h"
// #include "usb_dev.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
#if defined(__IMXRT1052__)
#include "debug/printf.h"
#undef printf
#endif
*/

extern void systick_isr(void);
extern volatile uint32_t systick_millis_count;
#define printf_debug pdbser1->printf

void userDebugDumptt() {
  volatile unsigned int nn;
  pdbser1->flush();
  faultPrintf("\n userDebugDumptt() in debug_tt  ___ \n", 0);
  pdbser1->flush();
#if defined(__IMXRT1052__)
  faultPrintf("\n F_CPU=%u", F_CPU_ACTUAL );
#endif
  systick_millis_count = 1;
  elapsedMicros systickEu = 0;

  debug_fault( DEBUG_T4 ); // DUMP OUT debug_tt TRACE LOG

  while (1)
  {
    if ( 1000 <= systickEu ) {
      systickEu -= 1000;
      systick_isr();
    }
    //GPIO2_DR_SET = (1 << 3); //
    digitalWrite(13, HIGH);
    // digitalWrite(13, HIGH);
    for (nn = 0; nn < 10000000; nn++) ;
    //GPIO2_DR_CLEAR = (1 << 3); //
    digitalWrite(13, LOW);
    // digitalWrite(13, LOW);
    pdbser1->flush();
    for (nn = 0; nn < 1800000; nn++) ;
    //    GPIO2_DR_SET = (1 << 3); //digitalWrite(13, HIGH);
    digitalWriteFast(13, HIGH);
    pdbser1->flush();
    for (nn = 0; nn < 10000000; nn++) ;
    //    GPIO2_DR_CLEAR = (1 << 3); //digitalWrite(13, LOW);
    digitalWriteFast(13, LOW);
    for (nn = 0; nn < 10000000; nn++) ;
    if ( !(millis() % 30)) {
      pdbser1->flush();
      faultPrintf(" @micros =%d" , micros());
      pdbser1->flush();
#if defined(__IMXRT1052__)
      faultPrintf( "\tdeg  C=%2.2f\n" , tempmonGetTemp() );
#endif
      pdbser1->flush();
    }
  }
}

#if defined(__IMXRT1052__)
uint32_t set_arm_clock(uint32_t frequency);

void HardFault_HandlerC(unsigned int *hardfault_args) {
  volatile unsigned int stacked_r0 ;
  volatile unsigned int stacked_r1 ;
  volatile unsigned int stacked_r2 ;
  volatile unsigned int stacked_r3 ;
  volatile unsigned int stacked_r12 ;
  volatile unsigned int stacked_lr ;
  volatile unsigned int stacked_pc ;
  volatile unsigned int stacked_psr ;
  volatile unsigned int _CFSR ;
  volatile unsigned int _HFSR ;
  volatile unsigned int _DFSR ;
  volatile unsigned int _AFSR ;
  volatile unsigned int _BFAR ;
  volatile unsigned int _MMAR ;
  volatile unsigned int addr ;
  volatile unsigned int nn ;

  stacked_r0 = ((unsigned int)hardfault_args[0]) ;
  stacked_r1 = ((unsigned int)hardfault_args[1]) ;
  stacked_r2 = ((unsigned int)hardfault_args[2]) ;
  stacked_r3 = ((unsigned int)hardfault_args[3]) ;
  stacked_r12 = ((unsigned int)hardfault_args[4]) ;
  stacked_lr = ((unsigned int)hardfault_args[5]) ;
  stacked_pc = ((unsigned int)hardfault_args[6]) ;
  stacked_psr = ((unsigned int)hardfault_args[7]) ;
  // Configurable Fault Status Register
  // Consists of MMSR, BFSR and UFSR
  _CFSR = (*((volatile unsigned int *)(0xE000ED28))) ;
  // Hard Fault Status Register
  _HFSR = (*((volatile unsigned int *)(0xE000ED2C))) ;
  // Debug Fault Status Register
  _DFSR = (*((volatile unsigned int *)(0xE000ED30))) ;
  // Auxiliary Fault Status Register
  _AFSR = (*((volatile unsigned int *)(0xE000ED3C))) ;
  // Read the Fault Address Registers. These may not contain valid values.
  // Check BFARVALID/MMARVALID to see if they are valid values
  // MemManage Fault Address Register
  _MMAR = (*((volatile unsigned int *)(0xE000ED34))) ;
  // Bus Fault Address Register
  _BFAR = (*((volatile unsigned int *)(0xE000ED38))) ;
  //__asm("BKPT #0\n") ; // Break into the debugger // NO Debugger here.

  if ( pdbser1 == NULL && Serial ) pdbser1 = (HardwareSerial *)&Serial;

  asm volatile("mrs %0, ipsr\n" : "=r" (addr)::);
  printf_debug("\ndebug_tt:: Fault irq %d\n", addr & 0x1FF);
  printf_debug(" stacked_r0 ::  %x\n", stacked_r0);
  printf_debug(" stacked_r1 ::  %x\n", stacked_r1);
  printf_debug(" stacked_r2 ::  %x\n", stacked_r2);
  printf_debug(" stacked_r3 ::  %x\n", stacked_r3);
  printf_debug(" stacked_r12 ::  %x\n", stacked_r12);
  printf_debug(" stacked_lr ::  %x\n", stacked_lr);
  printf_debug(" stacked_pc ::  %x\n", stacked_pc);
  printf_debug(" stacked_psr ::  %x\n", stacked_psr);
  printf_debug(" _CFSR ::  %x\n", _CFSR);
  printf_debug(" _HFSR ::  %x\n", _HFSR);
  printf_debug(" _DFSR ::  %x\n", _DFSR);
  printf_debug(" _AFSR ::  %x\n", _AFSR);
  printf_debug(" _BFAR ::  %x\n", _BFAR);
  printf_debug(" _MMAR ::  %x\n", _MMAR);


  if ( F_CPU_ACTUAL >= 600000000 )
    set_arm_clock(300000000);

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 5; // pin 13
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 = IOMUXC_PAD_DSE(7);
  GPIO2_GDIR |= (1 << 3);
  GPIO2_DR_SET = (1 << 3);
  GPIO2_DR_CLEAR = (1 << 3); //digitalWrite(13, LOW);

  userDebugDumptt();

  while (1)
  {
    GPIO2_DR_SET = (1 << 3); //digitalWrite(13, HIGH);
    // digitalWrite(13, HIGH);
    for (nn = 0; nn < 2000000 / 3; nn++) ;
    GPIO2_DR_CLEAR = (1 << 3); //digitalWrite(13, LOW);
    // digitalWrite(13, LOW);
    for (nn = 0; nn < 18000000; nn++) ;
  }
}
#endif


#ifdef __cplusplus
} // extern "C"
#endif


__attribute__((weak))
void Debug_Dump(void)
{
  printf_debug(" debug_tt (weak) default :: customize with 'void Debug_Dump()' in user code.");
  return;
}
