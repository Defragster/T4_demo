#define togPin 18
#define fStart 20000000
uint32_t iiF = fStart; 

// count ticks from external pin with  QTIMER4 chnl 2  pin 9 GPIO_B0_11
// test with PWM on pin togPin   jumper to 9

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
IMXRT_TMR_t * TMRx = (IMXRT_TMR_t *)&IMXRT_TMR4;


uint32_t prev;

IntervalTimer it1;
volatile uint32_t ticks, dataReady;

void it1cb() {
	ticks = TMRx->CH[2].CNTR | TMRx->CH[3].HOLD << 16; // atomic
	dataReady = 1;
	asm volatile("dsb");
}

void setup() {
	int cnt;

	while (!Serial && millis() < 4000 );
	Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
	delay(1000);

	analogWriteFrequency(togPin, iiF);  // test jumper 8 to 9, max 75mhz
	analogWrite(togPin, 128);

	CCM_CCGR6 |= CCM_CCGR6_QTIMER4(CCM_CCGR_ON); //enable QTMR4

	IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11 = 1;    // QT4 Timer2 on pin 9

	cnt = 65536 ; // full cycle
	TMRx->CH[2].CTRL = 0; // stop
	TMRx->CH[2].CNTR = 0;
	TMRx->CH[2].LOAD = 0;  // start val after compare
	TMRx->CH[2].COMP1 = cnt - 1;  // count up to this val and start again
	TMRx->CH[2].CMPLD1 = cnt - 1;
	TMRx->CH[2].SCTRL = 0;

	TMRx->CH[3].CTRL = 0; // stop
	TMRx->CH[3].CNTR = 0;
	TMRx->CH[3].LOAD = 0;  // start val after compare
	TMRx->CH[3].COMP1 = 0;
	TMRx->CH[3].CMPLD1 = 0;
	TMRx->CH[3].CTRL = TMR_CTRL_CM(7) | TMR_CTRL_PCS(6);  //clock from clock 2

	TMRx->CH[2].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH ;

	it1.begin(it1cb, 1000000);  // microseconds
}

uint32_t iiC = 3, iiN = 0;;

void loop() {
	if (dataReady) {
		if (prev == ticks) {
			analogWrite(togPin, 0);
			Serial.print( "   No Ticks ");
			iiN++;
			analogWrite(togPin, 128);
			if ( iiN > 5 ) {
				iiN = 0;
				iiF = fStart + 500000;
			}
		}
		Serial.print( "   ");
		Serial.print(ticks - prev);
		prev = ticks;
		dataReady = 0;
		iiC++;
		if ( !(iiC % 4) ) {
			iiF += fStart/10;
			analogWrite(togPin, 0);
			analogWriteFrequency(togPin, iiF);  // test jumper 8 to 9, max 75mhz
			analogWrite(togPin, 128);
			Serial.print("\nNEW Freq =");
			Serial.println(iiF);
		}
	}
}
