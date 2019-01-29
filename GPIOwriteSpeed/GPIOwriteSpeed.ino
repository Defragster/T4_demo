#define Oprint2( a, b ) ({ Serial1.print( a ); Serial1.print( b ); })
#define Oprint2l( a, b ) ({ Serial1.print( a ); Serial1.println( b ); })
#define print2( a, b ) ({ Serial.print( a ); Serial.print( b ); })
#define print2l( a, b ) ({ Serial.print( a ); Serial.println( b ); })

#define DiWr digitalWriteFast
//#define DiWr digitalWrite

uint32_t PinVar = LED_BUILTIN;
//#define PIN_NUM PinVar
//#define PIN_NUM 12 // NOT_LED
#define PIN_NUM LED_BUILTIN

void setup() {
  pinMode(PIN_NUM, OUTPUT);
}

bool Gflip = false;
void loop() {
  bool flip = Gflip;
  uint32_t start, end, foo;
  delay(1000);
  start = ARM_DWT_CYCCNT;
  DiWr(PIN_NUM, flip);
  flip = !flip;
  DiWr(PIN_NUM, flip);
  flip = !flip;
  DiWr(PIN_NUM, flip);
  flip = !flip;
  end = ARM_DWT_CYCCNT;
  Serial.print("flipped write >");
  Serial.println(end - start);
  delay(1000);
  start = ARM_DWT_CYCCNT;
  DiWr(PIN_NUM, Gflip);
  DiWr(PIN_NUM, !Gflip);
  DiWr(PIN_NUM, Gflip);
  end = ARM_DWT_CYCCNT;
  Serial.print("Global ! flip write >");
  Serial.println(end - start);
  delay(1000);
  start = ARM_DWT_CYCCNT;
  DiWr(PIN_NUM, HIGH);
  DiWr(PIN_NUM, LOW);
  DiWr(PIN_NUM, HIGH);
  end = ARM_DWT_CYCCNT;
  Serial.print("H L H 'const' write >");
  Serial.println(end - start);
  delay(1000);
  start = ARM_DWT_CYCCNT;
  DiWr(PIN_NUM, !digitalReadFast( PIN_NUM));
  DiWr(PIN_NUM, !digitalReadFast( PIN_NUM));
  DiWr(PIN_NUM, !digitalReadFast( PIN_NUM));
  end = ARM_DWT_CYCCNT;
  Serial.print("Three write(!ReadFast) >");
  Serial.println(end - start);
  delay(1000);
  DiWr(PIN_NUM, HIGH);
  start = ARM_DWT_CYCCNT;
  DiWr(PIN_NUM, !digitalRead( PIN_NUM));
  DiWr(PIN_NUM, !digitalRead( PIN_NUM));
  DiWr(PIN_NUM, !digitalRead( PIN_NUM));
  end = ARM_DWT_CYCCNT;
  Serial.print("Three write(!Read) >");
  Serial.println(end - start);
  delay(1000);
  foo = HIGH;
  start = ARM_DWT_CYCCNT;
  DiWr(PIN_NUM, foo);
  DiWr(PIN_NUM, foo);
  DiWr(PIN_NUM, foo);
  end = ARM_DWT_CYCCNT;
  Serial.print("H H H var write >");
  Serial.println(end - start);
  delay(1000);
  foo = LOW;
  start = ARM_DWT_CYCCNT;
  DiWr(PIN_NUM, foo);
  DiWr(PIN_NUM, foo);
  DiWr(PIN_NUM, foo);
  end = ARM_DWT_CYCCNT;
  Serial.print("L L L var write >");
  Serial.println(end - start);
  delay(1000);
  start = ARM_DWT_CYCCNT;
  DiWr(PIN_NUM, LOW);
  DiWr(PIN_NUM, HIGH);
  DiWr(PIN_NUM, LOW);
  end = ARM_DWT_CYCCNT;
  Serial.print("L H L 'const' write >");
  Serial.println(end - start);
  delay(1000);
  start = ARM_DWT_CYCCNT;
  DiWr(PIN_NUM, flip);
  DiWr(PIN_NUM, !flip);
  DiWr(PIN_NUM, flip);
  end = ARM_DWT_CYCCNT;
  Serial.print("! flip write >");
  Serial.println(end - start);
  Serial.println();
  delay(500);
  Gflip = flip;
}
