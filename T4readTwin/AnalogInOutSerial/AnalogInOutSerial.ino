/*
  Analog input, analog output, serial output

  Reads an analog input pin, maps the result to a range from 0 to 255 and uses
  the result to set the pulse width modulation (PWM) of an output pin.
  Also prints the results to the Serial Monitor.

  The circuit:
  - potentiometer connected to analog pin 0.
    Center pin of the potentiometer goes to the analog pin.
    side pins of the potentiometer go to +5V and ground
  - LED connected from digital pin 9 to ground

  created 29 Dec. 2008
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogInOutSerial
*/

// These constants won't change. They're used to give names to the pins used:
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = A21; // Analog output pin that the LED is attached to

int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  Serial2.begin(460800);
  pinMode(LED_BUILTIN, OUTPUT);
  while ( !Serial && millis() < 800 );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  analogWriteResolution(12);
}

char szbuff[30];
int oo;
void loop() {
  Tloop();
  Tloop();
  Tloop();
  Tloop();
  while(1);
}
void Tloop() {
  int jj=4;
/*  for ( int ii = 0; ii < 90; ii+=2 ) {
    outputValue = (4096*sin( ii*PI/180.0) );
  */
  for ( int ii = 0; ii <= 4096; ii+=40 ) {
    if ( jj > 0 ) { jj--; ii=0; }
    outputValue = ii;
    Serial2.print( 'r' );
    digitalWrite( LED_BUILTIN, !digitalRead( LED_BUILTIN ) );
    analogWrite(analogOutPin, outputValue );
    oo = 0;
    while ( !Serial2.available() ) {
    }
      while ( Serial2.available() ) {
        char ch = Serial2.read();
    digitalWrite( LED_BUILTIN, !digitalRead( LED_BUILTIN ) );
        // Serial.print( ch );
        if ( ch >= '0' && ch <= '9' )
          szbuff[oo++] = ch;
      }
    szbuff[oo] = 0;
    Serial.print( " T4 RECEIVE >>" );
    Serial.print( szbuff );
    // Serial.print( "    atol >>" );
    // Serial.println( atol(szbuff) );

    Serial.print("\t output = ");
    Serial.println(outputValue);
    delay(100);
    if ( ii == 4000 ) { ii=4096-80; }
  }
  Serial2.print( 'n' );

  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  Serial.println();
  delay(2000);
}
