// https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=194174&viewfull=1#post194174

#define HW_SERIAL Serial4 // pin 17 debug Tx only

volatile uint32_t jj = 0;

char szAlpha[26][65];

elapsedMillis noDelay;
void setup() {
  HW_SERIAL.print( "  setup() millis=" );
  HW_SERIAL.println( millis() );
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, HIGH );
  while ( !Serial );
  HW_SERIAL.print( "  Serial millis=" );
  HW_SERIAL.println( millis() );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  for ( int kk = 0; kk < 26; kk++ ) {
    for ( int ll = 0; ll < 64; ll++ )
      szAlpha[kk][ll] = kk + 'A';
    szAlpha[kk][63] = '\n';
    szAlpha[kk][64] = 0;
  }
  szAlpha[25][0] = '\t'; // Push Z's out to see then pass
  delayMicroseconds( 100 );
  noDelay = 0;
}

bool flip = true;
uint32_t ii = 0, kk;
int cc = 0;
void loop() {
  ii++;
  if ( Serial ) {
    if ( cc == 25 ) {
      Serial.print( szAlpha[cc] );
      cc = 0;
    }
    else {
      Serial.print( szAlpha[cc] );
      cc++;
    }
  }
  if ( noDelay > 1000 ) {
    noDelay -= 1000;
    digitalWrite( LED_BUILTIN, flip );
    flip = !flip;
    HW_SERIAL.print( "ITcnt=" );
    HW_SERIAL.println( jj );
    HW_SERIAL.print( "loop() ii=" );
    HW_SERIAL.print( ii );
    HW_SERIAL.print( "   micros=" );
    HW_SERIAL.println( micros() );
    jj = 0;
    ii = 0;
  }
}
