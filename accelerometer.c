#include <avr/io.h>
#include "accelerometer.h"
#include <util/delay.h>

void PWM_init(void) {
  // Timer 0
  TCCR0A = (1 << WGM00) | (1 << WGM01) // fast PWM
         | (1 << COM0A1) | (1 << COM0B1); // Clear OC0A/OC0B on compare
  TCCR0B = (1 << CS00); // No prescaler
  OCR0A = OCR0B = 0;

  // Timer 2
  TCCR2A = (1 << WGM20) | (1 << WGM21) // fast PWM
         | (1 << COM2A1) | (1 << COM2B1); // Clear OC2A/OC2B on compare
  TCCR2B = (1 << CS20); // No prescaler
  OCR2A =  OCR2B = 0;
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
      OCR2A = val;
      break;
    case 3:
      OCR2B = val;
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

  ADCSRA |= (1 << ADSC); // Start a single conversion

  while(!(ADCSRA & (1 << ADIF))); // Wait for conversion to complete

  ADCSRA |= (1 << ADIF); // Clear ADIF by writing one to it

  return(ADC);
}

void accel_init(uint8_t pin_x, uint8_t pin_y, uint8_t pin_z,
    uint16_t * avg_x, uint16_t * avg_y, uint16_t * avg_z) {
	uint8_t i;
	uint16_t temp;
	
	for (i = 0; i < 50; i++) {
		temp = ADC_read(pin_x);
		*avg_x = (*avg_x * (i) + temp) / (i + 1);
		
		temp = ADC_read(pin_y);
		*avg_y = (*avg_y * (i) + temp) / (i + 1);
		
		temp = ADC_read(pin_z);
		*avg_z = (*avg_z * (i) + temp) / (i + 1);

		_delay_ms(50);
	}
}

int16_t accel_get(int16_t *val, int16_t avg, uint8_t channel) {
  *val = ADC_read(channel) - avg;

  return *val;
}
