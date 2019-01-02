Starting Teensy 4 demo sketches for beta

EchoBoth - reads Serial1 to USB for debug on early T4 beta

BlinkMin - No delay blink with startup timing
--- output vals for reference
USB ::	Serial First millis=797 and !Serial wait loops ::    while Count ii=1311696
debug :: Into setup() > millis=306 and  Loop Hz 21394930

Blink_IntvTime - Running IntervalTimer and streaming strings out Serial USB 64 bytes per loop() or 1587136 bytes/sec
--- output vals for reference on Serial USB rows of alpha chars, with Z row pushed out with tab char
Count Interval Timer _isr ITcnt=498147  and passes through loop and running micros
loop() ii=24799   micros=12130230
