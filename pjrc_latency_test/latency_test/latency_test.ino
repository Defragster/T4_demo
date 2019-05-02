
void setup() {
  Serial.begin(115200);
  Serial.flush();
}

byte cck = 'A';
void loop() {
  while (Serial.available()) {
    byte c = Serial.read();
    if (c == 'x') {      // 'x' is end of input message
      Serial.write('0');
      Serial.write('1');
      Serial.write('2');
      Serial.write('x');
      Serial.send_now(); // comment out for non-Teensy boards
      cck = 'A';
    }
    else if (c != cck) {      // 'x' is end of input message
      Serial.write('B');
      Serial.write('A');
      Serial.write('D');
      Serial.write( c );
      Serial.send_now(); // comment out for non-Teensy boards
      while (Serial.available()) { // clear buffer for next test
        c = Serial.read();
        if (c == 'x') {      // 'x' is end of input message
          Serial.write('0');
          Serial.write('1');
          Serial.write('2');
          Serial.write('x');
          Serial.send_now(); // comment out for non-Teensy boards
          cck = 'A';
        }
      }

    }
    else if (++cck > 'Z') cck = 'A';
  }
}
