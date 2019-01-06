void setup() {
  // open a serial connection
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial5.begin(460800);
  while ( !Serial && millis() < 800 );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  //analogReadAveraging(32);

  // change the resolution to 8 bits and read A0
  aReadRes(12);

}

int aRead;
int aaRes = 12;
void loop() {

  // change the resolution to 12 bits and read A0
  while ( !Serial5.available() ) {
  }
  while ( Serial5.available() ) {
    char ch = Serial5.read();
    Serial.println(ch);
    digitalWrite( LED_BUILTIN, !digitalRead( LED_BUILTIN ) );
    if ( ch == 'n') {
      if ( 8 == aaRes )
        aaRes = 10;
      else if ( 10 == aaRes )
        aaRes = 12;
      else
        aaRes = 8;
      aReadRes(aaRes);
    }
    else if ( ch == 'r')
      aRead = analogRead(A0);
    Serial5.println(aRead);
    Serial.println(aRead);
  }
  //  Serial.println(analogRead(A0));
  // a little delay to not hog Serial Monitor
  delay(100);
}

void aReadRes(unsigned int bits)
{
  uint32_t tmp32, mode;
  Serial.print("ADC1_CFG: "); Serial.println(ADC1_CFG, BIN);

  if (bits == 8) {
    // 8 bit conversion (17 clocks) plus 8 clocks for input settling
    mode = ADC_CFG_MODE(0) | ADC_CFG_ADSTS(3);
  } else if (bits == 10) {
    // 10 bit conversion (17 clocks) plus 20 clocks for input settling
    mode = ADC_CFG_MODE(1) | ADC_CFG_ADSTS(2) | ADC_CFG_ADLSMP;
  } else {
    // 12 bit conversion (25 clocks) plus 24 clocks for input settling
    mode = ADC_CFG_MODE(2) | ADC_CFG_ADSTS(3) | ADC_CFG_ADLSMP;
  }
  tmp32  = (ADC1_CFG & ((1u << 22) - 1) << 10);
  tmp32 |= (ADC1_CFG & ((1u << 2) - 1) << 0); // ADICLK
  tmp32 |= ADC1_CFG & (((1u << 3) - 1) << 5); // ADIV & ADLPC

  tmp32 |= mode;
  ADC1_CFG = tmp32;

  Serial.print("ADC1_CFG: "); Serial.println(ADC1_CFG, BIN);

}


/*
  void aRAveraging(unsigned int num)
  {
  uint32_t tmp32, mode, avg=0;

  //disable averaging
  tmp32 = ADC1_GC;
  //Serial.println("ADC1_GC :"); Serial.println(ADC1_GC, BIN);

  ADC1_GC &= ~0x20;
  //Serial.print("Turn off ADC1_GC :"); Serial.println(ADC1_GC, BIN);
  //Serial.print("Initial ADC1_CFG :"); Serial.println(ADC1_CFG, BIN);

  mode = ADC1_CFG & ~0xC000;
  //Serial.print("Clear bits ADC1_CFG :"); Serial.println(mode, BIN);

    if (num >= 32) {
      mode |= ADC_CFG_AVGS(3);
      //Serial.println(ADC_CFG_AVGS(3), BIN);
    } else if (num >= 16) {
      mode |= ADC_CFG_AVGS(2);
    } else if (num >= 8) {
      mode |= ADC_CFG_AVGS(1);
    } else {
      mode |= ADC_CFG_AVGS(0);
    }

  ADC1_CFG = mode;
  //Serial.print("After avg set :"); Serial.println(ADC1_CFG, BIN);


  //enable averaging
  //ADC1_GC &= ~(ADC_GC_AVGE & 0x20);
  ADC1_GC = tmp32;
  //Serial.print("Turn on ADC_GC :"); Serial.println(ADC1_GC, BIN);
  }
*/
