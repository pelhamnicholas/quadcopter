#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Used for conversions and delays
#define F_CPU 8000000UL // 8 MHz
#include <util/delay.h>
#include "sonar.h"
#include "accelerometer.h"
/* DEBUGGING */
//#include "io.h"

#define microsecondsToCentimeters(a) ( (a) / 29 / 2 )

#define NUM_SONAR 4
#define NUM_MOTOR 4
#define MAX_RANGE 5 // maximum range detected by rangefinder

/* Motors */
#define FRONTRIGHT 0
#define BACKRIGHT 1
#define BACKLEFT 2
#define FRONTLEFT 3

/* Sonar */
#define FRONT 0
#define RIGHT 1
#define BACK 2
#define LEFT 3

/******************************************************************************
 * Motor Layout
 *   Front
 *   3   0
 *   2   1
 *   Back
 *
 * Sonar Layout
 *   Front
 *     0
 *   3   1
 *     2
 *   Back
 *
 * Accelerometer Axis
 *   x increases forward
 *   y increases left
 *   z increases up
 *
 *****************************************************************************/

int main(void) {
	DDRA=0x00; PORTA=0xFF; // PORTA = Accelerometer input
	DDRB=0xFF; PORTB=0x00; // PORTB = Motor output
	DDRC=0x00; PORTC=0xFF; // PORTC = Sonar Input
	DDRD=0xFF; PORTD=0x00; // PORTD = Sonar/Motor output
	/* DEBUGGING */
	//DDRC=0xFF; PORTC=0x00; // PORTC = LCD output

	int i;
	uint16_t range[NUM_SONAR];
	int16_t x, y, z, _x, _y, _z;
	uint16_t avg_x = 0, avg_y = 0, avg_z = 0;
	uint8_t motor[NUM_MOTOR];
	uint8_t motor_init;
	uint8_t obstacle;
	uint8_t stop_motors;
	/* DEBUGGING */
	//char debugString[32];

	ADC_init();
	PWM_init();
	/* DEBUGGING */
	//LCD_init();
	
	/* Initialize Accelerometer */
	x = y = z = 0;
	accel_init(2, 1, 0, &avg_x, &avg_y, &avg_z);
	_x = accel_get(&x, avg_x, 2);
	_y = accel_get(&y, avg_y, 1);
	_z = accel_get(&z, avg_z, 2);

	/* Initialize Motors */
	for (i = 0; i < NUM_MOTOR; i++)
		motor[i] = 0;
	motor_init = 1;
	stop_motors = 0;

	while(1) {
		/* GET DATA */
		// Accelerometer
		accel_get(&x, avg_x, 2);
		accel_get(&y, avg_y, 1);
		accel_get(&z, avg_z, 0);
		// Sonar
		obstacle = 0;
		for (i = 0; i < NUM_SONAR; i++) {
			range[i] = microsecondsToCentimeters(get_range_us(i, i));
			if (range[i] < MAX_RANGE)
				obstacle = 1;
			_delay_ms(5);
		}
		/* GET MOTOR SPEED */
		if (motor_init == 1) { // Takeoff
			for (i = 0; i < NUM_MOTOR; i++)
				if (motor[i] < 255)
					motor[i] = 0;
			//if (z > 15 || z < -15)
				motor_init = 0;
		} else if (obstacle == 1) { // Avoid
			/*if (range[0] < MAX_RANGE && range[2] < MAX_RANGE
					&& range[1] < MAX_RANGE && range[3] < MAX_RANGE) {
				stop_motors = 1;
			} else */if (range[0] < MAX_RANGE) {
				if (motor[1] >= 5)
					motor[1] -= 5;
				if (motor[2] >= 5)
					motor[2] -= 5;
				if (motor[0] <= 250)
					motor[0] += 5;
				if (motor[3] <= 250)
					motor[3] += 5;
			} else if (range[2] < MAX_RANGE) {
				if (motor[0] >= 5)
					motor[0] -= 5;
				if (motor[3] >= 5)
					motor[3] -= 5;
				if (motor[1] <= 250)
					motor[1] += 5;
				if (motor[2] <= 250)
					motor[2] += 5;
			}
			/*if (range[1] < MAX_RANGE && range[2] < MAX_RANGE) {
				stop_motors = 1;
			} else */if (range[1] < MAX_RANGE) {
				if (motor[2] >= 5)
					motor[2] -= 5;
				if (motor[3] >= 5)
					motor[3] -= 5;
				if (motor[0] <= 250)
					motor[0] += 5;
				if (motor[1] <= 250)
					motor[1] += 5;
			} else if (range[3] < MAX_RANGE) {
				if (motor[0] >= 5)
					motor[0] -= 5;
				if (motor[1] >= 5)
					motor[1] -= 5;
				if (motor[2] <= 250)
					motor[2] += 5;
				if (motor[3] <= 250)
					motor[3] += 5;
			}
		} else { // Hover
			if (x > 15) {
				if (motor[0] <= 250)
					motor[0] += 5; 
				if (motor[2] >= 5)
					motor[2] -= 5;
			} else if (x < 5) {
				if (motor[2] <= 250)
					motor[2] += 5;
				if (motor[0] >= 5)
					motor[0] -= 5;
			}
			if (y > 15) {
				if (motor[1] <= 250)
					motor[1] += 5;
				if (motor[3] >= 5)
					motor[3] -= 5;
			} else if (y < 5) {
				if (motor[3] <= 250)
					motor[3] += 5;
				if (motor[1] >= 5)
					motor[1] -= 5;
			}
			/*
			if (z > 20) {
				if (motor[0] < 250)
					motor[0] += 5;
				if (motor[1] < 250)
					motor[1] += 5;
				if (motor[2] < 250)
					motor[2] += 5;
				if (motor[3] < 250)
					motor[3] += 5;
			} else if (z < -2) {
				if (motor[0] > 5)
					motor[0] -= 5;
				if (motor[1] > 5)
					motor[1] -= 5;
				if (motor[2] > 5)
					motor[2] -= 5;
				if (motor[3] > 5)
					motor[3] -= 5;
			}
			*/
		}

		/* SET MOTOR SPEED */
		if (stop_motors)
			for (i = 0; i < NUM_MOTOR; i++)
				motor[i] = 0;
		for (i = 0; i < NUM_MOTOR; i++)
			PWM_set(i, motor[i]);

		/* DEBUGGING */
		//sprintf(debugString, "X%3d Y%3d Z%3d", x, y, z);
		//LCD_DisplayString(1, (unsigned char *) debugString);
		
		_delay_ms(20);
	}

	return(0);
}
