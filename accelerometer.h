#ifndef _ACCELEROMETER_H
#define _ACCELEROMETER_H

void PWM_init(void);
void PWM_set(uint8_t, uint8_t);

void ADC_init(void);
uint16_t ADC_read(uint8_t);

int16_t getVelocity(int16_t*, int16_t*, uint8_t);

void accel_init(uint8_t, uint8_t, uint8_t, uint16_t*, uint16_t*, uint16_t*);
int16_t accel_get(int16_t*, uint16_t, uint8_t);

#endif
