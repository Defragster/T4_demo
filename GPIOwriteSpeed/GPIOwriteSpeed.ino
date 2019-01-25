
#define DiWr digitalWriteFast
//#define DiWr digitalWrite

uint32_t PinVar = LED_BUILTIN;
#define NO_LED 12
//#define PIN_NUM NO_LED
#define PIN_NUM LED_BUILTIN
//#define PIN_NUM PinVar

void setup() {
  Serial1.begin( 115200 );
  Serial1.println("\n" __FILE__ " " __DATE__ " " __TIME__);
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
  Serial.print("Fault soon ...");
  delay(1000);
  Gflip = flip;
  GPT1_CNT = 5; // FAULT
}

#ifdef __cplusplus
extern "C" {
#endif
#include "debug/printf.h"
#undef printf
extern void systick_isr(void);
extern volatile uint32_t systick_millis_count;
#define printf_debug Serial1.printf

void userDebugDump() {
  volatile unsigned int nn;
  printf_debug("\n userDebugDump() IN SKETCH  ___ \n");
  Serial1.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  Serial1.printf("\n F_CPU=%u", F_CPU_ACTUAL );
  systick_millis_count = 1;
  elapsedMicros systickEu = 0;

  while (1)
  {
    if ( 1000 <= systickEu ) {
      systickEu -= 1000;
      systick_isr();
    }
    GPIO2_DR_SET = (1 << 3); //digitalWrite(13, HIGH);
    // digitalWrite(13, HIGH);
    for (nn = 0; nn < 10000000; nn++) ;
    GPIO2_DR_CLEAR = (1 << 3); //digitalWrite(13, LOW);
    // digitalWrite(13, LOW);
    for (nn = 0; nn < 1800000; nn++) ;
    //    GPIO2_DR_SET = (1 << 3); //digitalWrite(13, HIGH);
    digitalWriteFast(13, HIGH);
    for (nn = 0; nn < 10000000; nn++) ;
    //    GPIO2_DR_CLEAR = (1 << 3); //digitalWrite(13, LOW);
    digitalWriteFast(13, LOW);
    for (nn = 0; nn < 10000000; nn++) ;
    Serial1.printf("  micros =%d\n" , micros());
    Serial.printf( "   deg  C=%2.2f\n" , tempmonGetTemp() );
    Serial1.printf( "  deg  C=%2.2f\n" , tempmonGetTemp() );
    printf_debug( "  micros =%u\n" , micros() );
    printf_debug( "  millis =%u\n" , millis() );
  }
}

extern "C" uint32_t set_arm_clock(uint32_t frequency);

void HardFault_HandlerC(unsigned int *hardfault_args) {
  volatile unsigned int stacked_r0 ;
  volatile unsigned int stacked_r1 ;
  volatile unsigned int stacked_r2 ;
  volatile unsigned int stacked_r3 ;
  volatile unsigned int stacked_r12 ;
  volatile unsigned int stacked_lr ;
  volatile unsigned int stacked_pc ;
  volatile unsigned int stacked_psr ;
  volatile unsigned int _CFSR ;
  volatile unsigned int _HFSR ;
  volatile unsigned int _DFSR ;
  volatile unsigned int _AFSR ;
  volatile unsigned int _BFAR ;
  volatile unsigned int _MMAR ;
  volatile unsigned int addr ;
  volatile unsigned int nn ;

  stacked_r0 = ((unsigned int)hardfault_args[0]) ;
  stacked_r1 = ((unsigned int)hardfault_args[1]) ;
  stacked_r2 = ((unsigned int)hardfault_args[2]) ;
  stacked_r3 = ((unsigned int)hardfault_args[3]) ;
  stacked_r12 = ((unsigned int)hardfault_args[4]) ;
  stacked_lr = ((unsigned int)hardfault_args[5]) ;
  stacked_pc = ((unsigned int)hardfault_args[6]) ;
  stacked_psr = ((unsigned int)hardfault_args[7]) ;
  // Configurable Fault Status Register
  // Consists of MMSR, BFSR and UFSR
  _CFSR = (*((volatile unsigned int *)(0xE000ED28))) ;
  // Hard Fault Status Register
  _HFSR = (*((volatile unsigned int *)(0xE000ED2C))) ;
  // Debug Fault Status Register
  _DFSR = (*((volatile unsigned int *)(0xE000ED30))) ;
  // Auxiliary Fault Status Register
  _AFSR = (*((volatile unsigned int *)(0xE000ED3C))) ;
  // Read the Fault Address Registers. These may not contain valid values.
  // Check BFARVALID/MMARVALID to see if they are valid values
  // MemManage Fault Address Register
  _MMAR = (*((volatile unsigned int *)(0xE000ED34))) ;
  // Bus Fault Address Register
  _BFAR = (*((volatile unsigned int *)(0xE000ED38))) ;
  //__asm("BKPT #0\n") ; // Break into the debugger // NO Debugger here.

  asm volatile("mrs %0, ipsr\n" : "=r" (addr)::);
  printf_debug("\nin Sketch ... Fault irq %d\n", addr & 0x1FF);
  printf_debug(" stacked_r0 ::  %x\n", stacked_r0);
  printf_debug(" stacked_r1 ::  %x\n", stacked_r1);
  printf_debug(" stacked_r2 ::  %x\n", stacked_r2);
  printf_debug(" stacked_r3 ::  %x\n", stacked_r3);
  printf_debug(" stacked_r12 ::  %x\n", stacked_r12);
  printf_debug(" stacked_lr ::  %x\n", stacked_lr);
  printf_debug(" stacked_pc ::  %x\n", stacked_pc);
  printf_debug(" stacked_psr ::  %x\n", stacked_psr);
  printf_debug(" _CFSR ::  %x\n", _CFSR);
  printf_debug(" _HFSR ::  %x\n", _HFSR);
  printf_debug(" _DFSR ::  %x\n", _DFSR);
  printf_debug(" _AFSR ::  %x\n", _AFSR);
  printf_debug(" _BFAR ::  %x\n", _BFAR);
  printf_debug(" _MMAR ::  %x\n", _MMAR);

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 5; // pin 13
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 = IOMUXC_PAD_DSE(7);
  GPIO2_GDIR |= (1 << 3);
  GPIO2_DR_SET = (1 << 3);
  GPIO2_DR_CLEAR = (1 << 3); //digitalWrite(13, LOW);

  if ( F_CPU_ACTUAL >= 600000000 )
    set_arm_clock(300000000);

  printf_debug("\nHELLO WORLD ... Fault irq %d\n", addr & 0x1FF);
  userDebugDump();
  while (1)
  {
    GPIO2_DR_SET = (1 << 3); //digitalWrite(13, HIGH);
    // digitalWrite(13, HIGH);
    for (nn = 0; nn < 2000000; nn++) ;
    GPIO2_DR_CLEAR = (1 << 3); //digitalWrite(13, LOW);
    // digitalWrite(13, LOW);
    for (nn = 0; nn < 18000000; nn++) ;
  }
}


#ifdef __cplusplus
} // extern "C"
#endif
