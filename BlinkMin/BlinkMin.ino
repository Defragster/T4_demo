// https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=193998&viewfull=1#post193998
// https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=194174&viewfull=1#post194174

#define HW_SERIAL Serial4 // pin 17 debug Tx only

uint32_t SFirst = 0;
uint32_t ii = 0;
elapsedMillis noDelay;
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, HIGH );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  HW_SERIAL.print( "   millis=" );
  HW_SERIAL.println( millis() );
  Serial.print( "   millis=" );
  Serial.println( millis() );
  while ( !Serial ) {
    Serial.println( millis() );
    ii++;
  }
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  SFirst = millis();
  Serial.print( "   Serial First millis=" );
  Serial.println( SFirst );
  Serial.print( "   while Count ii=" );
  Serial.println( ii );
  delayMicroseconds( 100 );
  ii = 0;
  noDelay = 0;
}

bool flip = true;
void loop() {
  ii++;
  if ( noDelay > 1000 ) {
    noDelay -= 1000;
    digitalWrite( LED_BUILTIN, flip );
    flip = !flip;
    HW_SERIAL.print( " Loop Hz " );
    HW_SERIAL.println( ii );
    ii = 0;
  }
}
