/* UART Example, any character received on either the real
   serial port, or USB serial (or emulated serial to the
   Arduino Serial Monitor when using non-serial USB types)
   is printed as a message to both ports.

   This example code is in the public domain.
*/

// set this to the hardware serial port you wish to use
#define HWSERIAL Serial1
#define HWSERIALBAUD 1843200 // 115200

/* 
// Using the same on T4 Serial1 and Serial4 (PJRC DEBUG) requires:
T:\arduino-1.8.8T4_146\hardware\teensy\avr\cores\teensy4\debugprintf.c :: void printf_debug_init(void)
{
        CCM_CCGR0 |= CCM_CCGR0_LPUART3(CCM_CCGR_ON); // turn on Serial4
        IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_06 = 2; // Arduino pin 17
        // LPUART3_BAUD = LPUART_BAUD_OSR(25) | LPUART_BAUD_SBR(8); // ~115200 baud
    LPUART3_BAUD = LPUART_BAUD_OSR(12) | LPUART_BAUD_SBR(1);  // 1843200 baud

// for debug_tt to work when Faulted requires :: T:\arduino-1.8.8T4_146\hardware\teensy\avr\cores\teensy4\debugprintf.c
void HardwareSerial::flush(void)
{
  // bool bRetVal = transmitting_;
  int priority = nvic_execution_priority();
  if (priority <= hardware->irq_priority) {
    uint32_t head, tail, n;
    head = tx_buffer_head_;
    tail = tx_buffer_tail_;
    while (((port->WATER >> 8) & 0x7) < 4)
    {
      // bRetVal = true;
      if (head == tail) break;
      if (++tail >= tx_buffer_total_size_) tail = 0;
      if (tail < tx_buffer_size_) {
        n = tx_buffer_[tail];
      } else {
        n = tx_buffer_storage_[tail-tx_buffer_size_];
      }
      port->DATA = n;
    }
    tx_buffer_tail_ = tail;
    if (head == tail) {
      port->CTRL &= ~LPUART_CTRL_TIE; 
        port->CTRL |= LPUART_CTRL_TCIE; // Actually wondering if we can just leave this one on...
    }
  }
  else {
    while (transmitting_) yield(); // wait
  }
  // return bRetVal;
}


*/

void setup() {
  Serial.begin(9600);
  HWSERIAL.begin(HWSERIALBAUD);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, HIGH );
  while ( !Serial && millis() < 600 );
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  Serial.printf(" PORT Serial1 and BAUD=%d \n", HWSERIALBAUD );
}

void loop() {
  char incomingByte;

  if (Serial.available() > 0) {
    digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN) );
    while (Serial.available() > 0) {
      incomingByte = Serial.read();
      HWSERIAL.print(incomingByte);
    }
  }
  if (HWSERIAL.available() > 0) {
    digitalWriteFast( LED_BUILTIN, !digitalReadFast( LED_BUILTIN) );
    while (HWSERIAL.available() > 0) {
      incomingByte = HWSERIAL.read();
      Serial.print(incomingByte);
    }
  }
}
