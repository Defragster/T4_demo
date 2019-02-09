#include <TimerOne.h>

//#define DEBUG_OFF // defining this disables calls to debug library.
#include "debug_t3.h"

#define BUTTON_PIN 2 // Use button to GND this pin and loop() will Halt
#define BUTTON_ISR 3 // Use button to GND this pin and _isr() will Halt

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000 );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  DebState(LED_BUILTIN); // specify Blink Pin, pin is set to OUTPUT
  Timer1.initialize(10);
  Timer1.attachInterrupt( Timer_isr );
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ISR, INPUT_PULLUP);

  DebText( __func__ ); // show function entry
  deb_t3( 8, micros() );

  Serial.println("\n Setup DONE! All Good?");
}

uint32_t TimeTicks = 0;
void Timer_isr() {
  static uint16_t ii = 0;
  deb_t3( ii, micros() );
  haltif_t3( digitalReadFast( BUTTON_ISR ) ); // Stop on button press
  ii++;
  ii &= 7;
  TimeTicks++;
  deb_t3( 9, TimeTicks );
}

elapsedMillis eMblink;
void loop() {
  haltif_t3( digitalReadFast( BUTTON_PIN ) ); // Stop on button press
  if ( eMblink>=1000 ) {
    eMblink = 0;
    qBlink();
    
  }
}

// For haltif() entering 'd' calls this optional code
// It can call functions or dump globals.
void Debug_Dump(void)
{
  DebSer.print(" User Custom Debug Dump. Micros==");
  DebSer.println(micros());
  DebSer.print("\t _isr TimeTicks==");
  DebSer.println(TimeTicks);
  return;
}


