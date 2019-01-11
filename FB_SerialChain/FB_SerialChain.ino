// LOOP Rx to Tx on Each port in the test
// To use Serial4 Disable Debug :: https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=195229&viewfull=1#post195229
#define SPD   5333333
#define FMT SERIAL_8N1
char outstring[] = "0123456789a123456789b123456789c123456789d123456789E123456789f123456789\n";

void setup() {
  while ( !Serial && millis() < 800 ) ;
  pinMode(13, OUTPUT);
  Serial.println("Hello world");
  Serial.println(sizeof(outstring) - 1);

  Serial1.begin(SPD, FMT);
  Serial2.begin(SPD, FMT);
  Serial3.begin(SPD, FMT);
  Serial4.begin(SPD, FMT);
  Serial5.begin(SPD, FMT);
  Serial6.begin(SPD, FMT);
  Serial7.begin(SPD, FMT);
  Serial8.begin(SPD, FMT);
  Serial1.print( outstring );
}

int ii = 0, kk = 0, cc=0;
char foo[200];
void loop() {
  char ch = 0;
  if (Serial1.available()) Serial2.write(Serial1.read());
  if (Serial2.available()) Serial3.write(Serial2.read());
  if (Serial3.available()) Serial4.write(Serial3.read()); // disable debug //#define PRINT_DEBUG_STUFF
  if (Serial4.available()) Serial5.write(Serial4.read());
  if (Serial5.available()) Serial6.write(Serial5.read());
  if (Serial6.available()) Serial7.write(Serial6.read());
  if (Serial7.available()) Serial8.write(Serial7.read());
  if (Serial8.available()) {
    foo[ii++] = ch = Serial8.read();
    Serial1.write( ch );
  }
  if ( ch == '\n' ) {
    cc++;
    if ( (sizeof(outstring) - 1) != ii ) {
      if ( ii > sizeof(foo) ) ii = sizeof(foo) - 1;
      foo[ii] = 0;
      Serial.println( " fail ");
      Serial.print( foo );
      delay( 500 );
    }
    Serial.print( ii);
    kk++;
    if ( kk >= 50 ) {
      digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN ) );
      kk = 0;
      Serial.println();
      foo[ii] = 0;
      Serial.print( cc );
      Serial.print( " @" );
      Serial.print( millis() );
      Serial.print( " " );
      Serial.print( foo );
      delay(10);
    }
    if ( (sizeof(outstring) - 1) != ii )
      Serial1.print( outstring );
    ii = 0;
  }
}
