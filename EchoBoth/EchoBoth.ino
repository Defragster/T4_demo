/* UART Example, any character received on either the real
   serial port, or USB serial (or emulated serial to the
   Arduino Serial Monitor when using non-serial USB types)
   is printed as a message to both ports.

   This example code is in the public domain.
*/

// set this to the hardware serial port you wish to use
#define HWSERIAL Serial1
#define HWSERIALBAUD 115200

void setup() {
  Serial.begin(9600);
  HWSERIAL.begin(HWSERIALBAUD);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, HIGH );
  while ( !Serial && millis() < 600 );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  Serial.printf(" PORT Serial1 and BAUD=%d \n", HWSERIALBAUD );
}

void loop() {
  char incomingByte;

  if (Serial.available() > 0) {
    digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN) );
    while (Serial.available() > 0) {
      incomingByte = Serial.read();
      HWSERIAL.println(incomingByte);
    }
  }
  if (HWSERIAL.available() > 0) {
    digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN) );
    while (HWSERIAL.available() > 0) {
      incomingByte = HWSERIAL.read();
      Serial.print(incomingByte);
    }
  }
}
