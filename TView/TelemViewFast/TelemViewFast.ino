
uint32_t usDelay = 10; // start us delay
//#define DELAY_DROP 1 // comment to keep a static delay

uint32_t waveform_a;
uint32_t count = 0;
void setup() {
  Serial.begin(9600);
  Serial2.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(14, OUTPUT); // Change value with each output string for FreqCount
  Serial2.print( "Starting with delayMicroseconds of=" );
  Serial2.println( usDelay );
  while ( !Serial ) {
    digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN ) );
    delay(250);
    digitalWriteFast( 14, 1 );
    delay(250);
    digitalWriteFast( 14, 0 );
  }
  waveform_a = usDelay;
}

void loop() {
  count++;
#ifdef DELAY_DROP
  if ( !(count % 60000) ) {
    if ( usDelay > 45 )
      usDelay -= 5;
    else if ( usDelay > 5 )
      usDelay -= 2;
  waveform_a = usDelay;
  }
#endif
  digitalWriteFast( LED_BUILTIN, (count & 0x800) && 0x800 );
  digitalWriteFast( 14, count & 1 );
  uint32_t waveform_b = ((count & 0x3ffF0) >> 4);
  uint32_t waveform_c = ((count & 0x3ff));
  uint32_t sine_wave_1khz = 1000.0 * sin(radians( count % 90 ));

  Serial.printf("%u,%u,%u,%u\n", (int)waveform_a, (int)waveform_b, (int)waveform_c, (int)sine_wave_1khz);
  delayMicroseconds(usDelay);
}
