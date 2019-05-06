/* UART Example, any character received on either the real
   serial port, or USB serial (or emulated serial to the
   Arduino Serial Monitor when using non-serial USB types)
   is printed as a message to both ports.

   This example code is in the public domain.
*/
#include "FreqCount.h"
const int fCntPin = 13; // Teensy FreqCount uses this pin
const int isrDbgPin = 4; // Pin Change interrupt message here

// set this to the hardware serial port you wish to use
#define HWSERIAL Serial1
#define HWSERIAL2 Serial2
//#define HWSERIALBAUD 1843200 // 921600 // 115200
//#define HWSERIALBAUD  2000000
#define HWSERIALBAUD  115200
volatile boolean TurnDetected;  // need volatile for Interrupts
void isrDbgFunc()  {
  TurnDetected = true;  // set variable to true
}

void setup() {
  Serial.begin(9600);
  HWSERIAL.begin(HWSERIALBAUD);
  HWSERIAL2.begin(HWSERIALBAUD);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(isrDbgPin, INPUT);
  FreqCount.begin(1000); // Count per second 1000 ms
  digitalWrite( LED_BUILTIN, HIGH );
  attachInterrupt(isrDbgPin, isrDbgFunc, CHANGE);
  while ( !Serial && millis() < 2000 );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  Serial.printf(" PORT Serial1 and BAUD=%d \n", HWSERIALBAUD );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  Serial.printf(" PORT Serial2 and BAUD=%d \n", HWSERIALBAUD );
}

char buff[200];
void loop() {
  char incomingByte;
  static long lastFcnt = -1;

  if (FreqCount.available()) {
    unsigned long count = FreqCount.read();
    if ( lastFcnt != count) {
      Serial.print("fCnt=");
      Serial.println(count);
      lastFcnt = count;
    }
  }

  if (TurnDetected)  {
    TurnDetected = false;
    Serial.print("XXX isr from Debug Client! XXX  ");
    Serial.println(micros());
    HWSERIAL.println("XXX isr from Debug Client! XXX  ");
  }

  if (Serial.available() > 0) {
    digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN) );
    while (Serial.available() > 0) {
      incomingByte = Serial.read();
      HWSERIAL.print(incomingByte);
    }
  }
  if (HWSERIAL.available() > 0) {
    int ii = HWSERIAL.available();
    digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN) );
    while (HWSERIAL.available() > 0) {
      HWSERIAL.readBytes( buff, ii );
      buff[ii] = 0;
      Serial.print(buff);
    }
  }

  if (HWSERIAL2.available() > 0) {
    int ii = HWSERIAL2.available();
    digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN) );
    while (HWSERIAL2.available() > 0) {
      HWSERIAL2.readBytes( buff, ii );
      buff[ii] = 0;
      Serial.print(buff);
    }
  }
}
