

#ifndef _debug_ttc_
#define _debug_ttc_

#ifndef NO_DEBUG_tt
extern uint32_t lastL_tt;
extern char * lastF_tt;

#define assert_tt( a ) if (!(a)) { assert_ttf(__FILE__, __LINE__,  #a, __func__); lastF_tt=(char*)__func__; lastL_tt=__LINE__; }
#define haltif_tt( a ) if ((a)) { haltif_ttf(__FILE__, __LINE__,  #a, __func__); lastF_tt=(char*)__func__; lastL_tt=__LINE__; }
#define debTrace_tt( a, b, c ) { debTrace_ttf( (uint32_t) a, (uint32_t) b, (const char *) c, __LINE__, __func__); lastF_tt=(char*)__func__; lastL_tt=__LINE__; }
#define deb_tt( a, b ) {debSet( a, (uint32_t)b, __LINE__, __func__ ); lastF_tt=(char*)__func__; lastL_tt=__LINE__; }

void debTraceShow_tt( int Max, const char *aa, const char *bb, const char *cc);

// Used by Macro
void debSet( uint16_t a, uint32_t b, uint16_t lineN, const char *__func ); 
void assert_ttf(const char *__file, int __lineno, const char *__sexp, const char *__func );
void haltif_ttf(const char *__file, int __lineno, const char *__sexp, const char *__func );
void debTrace_ttf( uint32_t aa, uint32_t bb, const char *cc, int __lineno, const char *__func);
void DebugBlink( uint32_t wait );
void dPromptUser();
// not used?
void debug_fault( int iFrom );
void resetReason( int resetReasonHw );

#else
#define assert_tt( a ) 
#define haltif_tt( a ) 
#define debTrace_tt( a, b, c ) 
#define deb_tt( a, b ) 
#define debTraceShow_tt( a, b, c, d )

#endif
#endif
