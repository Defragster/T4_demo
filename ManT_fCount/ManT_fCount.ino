#include <FreqCount.h>
IntervalTimer myTimer;
#define togPin 18
#define FreqPin 9
#define fStart 20000000
uint32_t iiF = fStart;

volatile uint32_t iiC = 3, iiN = 0;;
uint32_t lFreq = 0;
volatile uint32_t dataReady;
void getFreq() {
	if ( FreqCount.available() ) {
		lFreq = FreqCount.read();
		dataReady = 1;
	}
}

elapsedMillis fWDog = 0;
void setup() {
	while (!Serial && millis() < 4000 );
	Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
	delay(1000);

	analogWriteFrequency(togPin, iiF);  // test jumper togPin to 9, max 75mhz
	analogWrite(togPin, 128);
	myTimer.begin(getFreq, 1000000);  // check freq each second
	FreqCount.begin(1000000);  //Time in microseconds
	fWDog = 0;
	Serial.println("\n end setup" );
}

void loop() {
	if (dataReady || fWDog > 1010) {
		if (fWDog > 1010 || 0 == lFreq) {
			analogWrite(togPin, 0);
			Serial.print( "   No Ticks ");
			iiN++;
			analogWrite(togPin, 128);
			if ( iiN > 6 ) {
				iiN = 0;
				iiF = fStart + 500000;
			}
		}
		fWDog = 0;
		Serial.print( "   ");
		Serial.print( lFreq );
		dataReady = 0;
		iiC++;
		if ( !(iiC % 5) ) {
			iiF += fStart / 10;
			analogWrite(togPin, 0);
			analogWriteFrequency(togPin, iiF);  // test jumper 8 to 9, max 75mhz
			analogWrite(togPin, 128);
			Serial.print("\nNEW Freq =");
			Serial.println(iiF);
		}
	}
}
