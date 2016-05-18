#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
// Used for conversions and delays
#define F_CPU 8000000UL // 8 MHz
#include <util/delay.h>

// From Arduino.h
#define clockCyclesPerMicrosecond ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond )

#define microsecondsToInches(a) ( (a) / 74 / 2 )
#define microsecondsToCentimeters(a) ( (a) / 29 / 2 )

typedef struct rangefinder {
	//unsigned char port; Assumes all rangefinders are on PORTD
	uint8_t trig;
	uint8_t echo;
} rangefinder;

void RF_init(rangefinder * rf, uint8_t trigger, uint8_t echo) {
	DDRD |= (1 << trigger); PORTD &= ~(1 << trigger);
	DDRD &= ~(1 << echo); PORTD |= (1 << echo);
	
	rf->trig = trigger;
	rf->echo = echo;
}

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

// Assumes all rangefinders are on PORTD
uint16_t get_range(rangefinder *rf) {
	uint16_t maxloops = microsecondsToClockCycles(50) / 16;
	uint16_t numloops = 0;
	uint16_t width = 0;
	
	// Send out pulse
	PORTD &= 0xFE;//rf->trig);
	// delay 2 us
	_delay_us(2);
	PORTD |= 0x01;//rf->trig);
	// delay 5 us
	_delay_us(15);
	PORTD &= ~0xFE;//rf->trig);
	
	// get echo pulse
	while ((PIND & (1 << rf->echo)) == 1) {
		if (numloops++ == maxloops) {
			return -1;
		}
	}
	
	while ((PIND & (1 << rf->echo)) == 0) {
		if (numloops++ == maxloops) {
			return -1;
		}
	}
	
	while ((PIND & (1 << rf->echo)) == 1) {
		if (numloops++ == maxloops) {
			return -1;
		}
		width++;
	}
	
	return width;//clockCyclesToMicroseconds(width * 21 + 16);
}

void PWM_init(void) {
	TCCR0A = (1 << WGM00) | (1 << WGM01) // fast PWM
	| (1 << COM0A1) | (1 << COM0B1); // Clear OC0A/OC0B on compare
	TCCR0B = (1 << CS00); // No prescaler
	OCR0A = OCR0B = 0;

	TCCR3A = (1 << WGM30) | (1 << WGM31) // fast PWM
	| (1 << COM3A1) | (1 << COM3B1); // Clear OC0A/OC0B on compare
	TCCR3B = (1 << CS30); // No prescaler
	OCR3A =  OCR3B = 0;
}

void PWM_set(uint8_t channel, uint8_t val) {
	switch (channel) {
		case 0:
		OCR0A = val;
		break;
		case 1:
		OCR0B = val;
		break;
		case 2:
		OCR3A = val;
		break;
		default:
		break;
	}
}

void ADC_init(void) {
	ADMUX = (1 << REFS0); // For Aref = AVcc;
	// Prescalar div factor = 128;
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read(uint8_t channel) {
	// Select ADC Channel
	channel &= 0x07;
	ADMUX = (ADMUX & 0xF8) | channel;

	// Start a single conversion
	ADCSRA |= (1 << ADSC);

	// Wait for conversion to complete
	while(!(ADCSRA & (1 << ADIF)));

	// Clear ADIF by writing one to it
	ADCSRA |= (1 << ADIF);

	return(ADC);
}

int16_t getVelocity(int16_t *velocity, int16_t *last_val, uint8_t channel) {
	int16_t delta_v;

	delta_v = ADC_read(channel);
	//_delay_ms(1);
	delta_v = ADC_read(channel) - delta_v;//last_val;

	*velocity += delta_v;//(delta_v - *last_val) * 100;
	
	//*last_val = delta_v;

	return *velocity;
}

void XLRMTR_init(uint16_t * avg_x, uint16_t * avg_y, uint16_t * avg_z) {
	uint8_t i;
	uint16_t temp;
	
	for (i = 1; i < 50; i++) {
		temp = ADC_read(2);
		*avg_x = (*avg_x * (i - 1) + temp) / i;
		
		temp = ADC_read(1);
		*avg_y = (*avg_y * (i - 1) + temp) / i;
		
		temp = ADC_read(0);
		*avg_z = (*avg_z * (i - 1) + temp) / i;

		_delay_ms(10);
	}
}

int16_t XLRMTR_get(int16_t *val, uint16_t avg, uint8_t channel) {
	*val = ADC_read(channel) - avg;
	
	return *val;
}

int main(void) {
	DDRA=0xF0; PORTA=0x0F; // PORTA = output/input
	DDRB=0xFF; PORTB=0x00; // PORTB = output
	DDRC=0xFF; PORTC=0x00; // PORTC = output
	DDRD=0xFF; PORTD=0x00; // PORTD = output

	int i;
	//uint16_t _ADC;
	char outStr[32], tmpStr[20];
	rangefinder rf;
	uint16_t range_cm;
	int16_t x, y, z;
	uint16_t avg_x = 0, avg_y = 0, avg_z = 0;

	ADC_init();
	PWM_init();
	LCD_init();
	RF_init(&rf, 0, 2);
	
	XLRMTR_init(&avg_x, &avg_y, &avg_z);
	
	// circuit digest
	// altered for atmega1284
	EIMSK |= (1 << INT0);
	EICRA |= (1 << ISC00);
	TCCR1A = 0;
	sei();

	while(1) {
		//tempC = 0x00;
		outStr[0] = '\0';
		
		// accelerometer
		for(i = 2; i >= 0; i--) {
			switch(i) {
				case 0:
				strcat(outStr, " Z = ");
				XLRMTR_get(&z, avg_z, i);
				sprintf(tmpStr, "%3d", z);
				break;
				case 1:
				strcat(outStr, " Y = ");
				XLRMTR_get(&y, avg_y, i);
				sprintf(tmpStr, "%3d", y);
				break;
				case 2:
				strcat(outStr, "X = ");
				XLRMTR_get(&x, avg_x, i);
				sprintf(tmpStr, "%3d", x);
				default:
				break;
			}
			//_ADC = ADC_read(i);
			//itoa(_ADC, tmpStr, 10);
			strcat(outStr, tmpStr);
		}
		
		// rangefinder
		// Software
		//range_us = get_range(&rf);
		//range_cm = microsecondsToCentimeters(range_us);
		// Hardware
		PORTD |= 0x01;
		_delay_us(15);
		PORTD &= 0xFE;
		range_cm = microsecondsToCentimeters(pulse);
		
		strcat(outStr, " R =");
		//itoa(range_cm, tmpStr, 10);
		sprintf(tmpStr, "%3d", range_cm);
		strcat(outStr, tmpStr);
		strcat(outStr, "cm");
		
		// output
		LCD_DisplayString(1, (unsigned char *)outStr);
		//PWM_set(1, ADC_read(_ADC >> 2));
		
		_delay_ms(100);
	}

	return(0);
}
