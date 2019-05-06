// https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=204883&viewfull=1#post204883
// https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=204681&viewfull=1#post204681
uint32_t count, prior_count;
uint32_t prior_msec;
uint32_t count_per_second;

void setup() {
  Serial.begin(1000000);
  Serial2.begin(115200);
  Serial2.print( "Hello World" );
  pinMode(14, OUTPUT); // Change value with each output string for FreqCount
  //  while (!Serial) ;
  count = 10000000;
  prior_count = count;
  count_per_second = 0;
  prior_msec = millis();
}
void yield() {}
void loop() {
  char c;
  int ii = 0;
  char buf[100];
  while (Serial.available()) {
    c = Serial.read();
    if (c == '\n') {
      count = count + 1;
      digitalWriteFast( 14, count & 1 );
      uint32_t msec = millis();
      if (msec - prior_msec > 1000) {
        prior_msec = prior_msec + 1000;
        count_per_second = count - prior_count;
        prior_count = count;
        buf[ii] = 0;
        Serial2.print(buf);
#if 1 // this is showing echo on print of last digits of count_per_second and newlines
        Serial2.println(count_per_second);
#else
        Serial2.print(count_per_second);
        Serial2.println('.');
#endif
        ii = 0;
      }
    }
    else {
      buf[ii++] = c;
    }
  }
}
