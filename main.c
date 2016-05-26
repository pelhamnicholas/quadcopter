#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
// Used for conversions and delays
#define F_CPU 8000000UL // 8 MHz
#include <util/delay.h>


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
	sonar_init(0, 2);
	
	accel_init(&avg_x, &avg_y, &avg_z);
	
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
				accel_get(&z, avg_z, i);
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
		pulse = 0;
		
		strcat(outStr, " R =");
		//itoa(range_cm, tmpStr, 10);
		sprintf(tmpStr, "%3d", range_cm);
		strcat(outStr, tmpStr);
		strcat(outStr, "cm");
		
		// output
		LCD_DisplayString(1, (unsigned char *)outStr);
		//PWM_set(1, ADC_read(_ADC >> 2));
		
		_delay_ms(500);
	}

	return(0);
}
