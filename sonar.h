#ifndef _SONAR_H_
#define _SONAR_H_

#define F_CPU 8000000UL // 8 MHz
// From Arduino.h
#define clockCyclesPerMicrosecond ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond )
//#define millisecondsToClockCycles(a) ( microsecondsToClockCycles(a) * 1000 )
#define microsecondsToCentimeters(a) ( (a) / 29 / 2 )

static inline uint16_t millisecondsToClockCycles(uint16_t a) {
  return microsecondsToClockCycles(a) * 1000;
}

/*
// From circuit digest
static volatile int pulse = 0;
static volatile int i = 0;

ISR(INT0_vect) {
	if (i == 1) {
		TCCR1B = 0;
		pulse = TCNT1 / 8;
		TCNT1 = 0;
		i = 0;
	}
	if (i == 0) {
		TCCR1B |= (1 << CS10);
		i = 1;
	}
}
*/

// Assumes all rangefinders are on PORTD
uint16_t get_range_us(uint8_t, uint8_t); 
void sonar_init(uint8_t, uint8_t);

#endif
