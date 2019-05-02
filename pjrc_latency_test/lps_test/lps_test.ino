// https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=204681&viewfull=1#post204681
uint32_t count, prior_count;
uint32_t prior_msec;
uint32_t count_per_second;

// Uncomment this for boards where SerialUSB needed for native port
//#define Serial SerialUSB

void setup() {
  Serial.begin(1000000);
  while (!Serial) ;
  count = 10000000;
  prior_count = count;
  count_per_second = 0;
  prior_msec = millis();
}

void loop() {
  Serial.print("count=");
  Serial.print(count);
  Serial.print(", lines/sec=");
  Serial.println(count_per_second);
  count = count + 1;
  uint32_t msec = millis();
  if (msec - prior_msec > 1000) {
    prior_msec = prior_msec + 1000;
    count_per_second = count - prior_count;
    prior_count = count;
  }
}
