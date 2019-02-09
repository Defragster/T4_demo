
#ifndef _debug_tt_

#define _debug_tt_

//#define DEBUG_OFF

#define NO_BLINK 255 // disable blink toggle
#define NO_ISR 255 // disable blink toggle

#ifndef DEBUG_OFF
extern HardwareSerial *pdbser1;


#define where_tt( ) { if(1) {int where_ii; pdbser1->print( __func__); pdbser1->print( __LINE__); pdbser1->print( "() stack var: " ); pdbser1->println ( (uint32_t)&where_ii,HEX ); lastF_tt=(char*)__func__; lastL_tt=__LINE__;} }
#define addr_tt( a ) { pdbser1->print( #a); pdbser1->print( " addr: " ); pdbser1->println ( (uint32_t)a,HEX ); lastF_tt=(char*)__func__; lastL_tt=__LINE__; }


// #define assert_ttx( a ) do {if (!a) { assert_ttf(__FILE__, __LINE__,  #a) } while(0)

// TODO -- Make these wrappers for _ttf's for C udage?
#define debText_tt( a )  { pdbser1->print("_:L="); pdbser1->print(__LINE__ ); pdbser1->print( ": " ); pdbser1->print(a); pdbser1->println(); lastF_tt=(char*)__func__; lastL_tt=__LINE__; }
#define debTextH_tt( a )  { pdbser1->print("_:L="); pdbser1->print(__LINE__ ); pdbser1->print( ": " ); pdbser1->println(a, HEX); lastF_tt=(char*)__func__; lastL_tt=__LINE__; }
#define debText2_tt( a, b )   { pdbser1->print("_:L="); pdbser1->print(__LINE__ ); pdbser1->print( ": " ); pdbser1->print(a); pdbser1->print(b); pdbser1->println(); lastF_tt=(char*)__func__; lastL_tt=__LINE__; }

extern void Debug_Dump(void);

//#define qBlink() (GPIOC_PTOR = 32)  // Pin13 on T3.x & LC // requires: pinMode(LED_BUILTIN, OUTPUT);
#define qBlink() digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN ) )

#ifdef __cplusplus
extern "C" {
#endif

uint16_t debBegin_tt(  HardwareSerial *pserial, uint16_t DoBlink, uint32_t DoIsr );

#include "debug_ttc.h"

#ifdef __cplusplus
} // extern "C"
#endif

// #define XXX_YYY  do{ ii++ }while(0)
// #define qBlinkT( a )  do{ (GPIOC_PTOR = 32) }while( a != digitalReadFast( LED_BUILTIN) )

#else // DEBUG_OFF
#define deb_tt( a, b ) // debSet( a, b, __LINE__, __func__ );

#define debSet( a,b,c,d )
//#define DebState( a ) 0
#define debBegin_tt(  a, b, c );

#define assert_tt( a ) // if (!a) { assert_ttf(__FILE__, __LINE__,  #a, __func__); }
#define haltif_tt( a )
#define debTrace_tt( a, b, c )
#define where_tt( )
#define addr_tt( a )

#define debText_tt( a )
#define DebTextH_tt( a )
#define debText2_tt( a, b )

#define debug_fault( a )

//#define qBlink() (GPIOC_PTOR = 32)  // Pin13 on T3.x & LC // requires: pinMode(LED_BUILTIN, OUTPUT);
#define qBlink() digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN ) )
#define assert_ttf( a,b,c,d )
#define DebugWait( a )

#endif
#endif
