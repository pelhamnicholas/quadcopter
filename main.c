#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Used for conversions and delays
#define F_CPU 8000000UL // 8 MHz
#include <util/delay.h>
#include "timer.h"
#include "task.h"
#include "sonar.h"
#include "accelerometer.h"
/* DEBUGGING */
//#include "io.h"

#define microsecondsToCentimeters(a) ( (a) / 29 / 2 )

#define NUM_SONAR 4
#define NUM_MOTOR 4
#define MAX_RANGE 5 // maximum range detected by rangefinder

/* Accelerometer */
#define PINX 2
#define PINY 1
#define PINZ 0

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

/* Shared Variables */
int16_t x, y, z;
uint8_t obstacle;
uint16_t range[NUM_SONAR];
uint8_t motor[NUM_MOTOR];

enum accel_states { accel_initstate, accel_offset };

int accel_tick(int state) {
  static uint16_t avg_x, avg_y, avg_z;

  switch(state) {
    case accel_initstate:
      avg_x = avg_y = avg_z = 0;
      accel_init(PINX, PINY, PINZ, &avg_x, &avg_y, &avg_z);
      break;
    case accel_offset:
      accel_get(&x, avg_x, PINX);
      accel_get(&y, avg_y, PINY);
      accel_get(&z, avg_z, PINZ);
      break;
    default:
      break;
  }

  switch(state) {
    case accel_initstate:
      state = accel_offset;
      break;
    case accel_offset:
      state = accel_offset;
      break;
    default:
      break;
  }

  return state;
}

enum sonar_state { sonar_get_dist };

int sonar_tick(int state) {
  int i;

  obstacle = 0;

  switch(state) {
    case sonar_get_dist:
      for (i = 0; i < NUM_SONAR; i++) {
        range[i] = microsecondsToCentimeters(get_range_us(i, i));
        if (range[i] < MAX_RANGE)
          obstacle = 1;
        _delay_ms(5);
      }
    default:
      break;
  }

  switch(state) {
    case sonar_get_dist:
    default:
      state = sonar_get_dist;
      break;
  }

  return state;
}

enum mcontrol_state { mcontrol_init, mcontrol_takeoff, mcontrol_hover,
                           mcontrol_avoid, mcontrol_killed };

int mcontrol_tick(int state) {
  static uint8_t motor[NUM_MOTOR];
  static uint8_t killed;
  int i;

  switch(state) {
    case mcontrol_init:
      for (i = 0; i < NUM_MOTOR; i++) {
        motor[i] = 0;
      }
      killed = 0;
      break;
    case mcontrol_takeoff:
			for (i = 0; i < NUM_MOTOR; i++) {
				if (motor[i] < 255) {
					motor[i] = 0;
        }
      }
      break;
    case mcontrol_hover:
			if (x > 15) { // should be > 5
				if (motor[0] <= 250)
					motor[0] += 5; 
				if (motor[2] >= 5)
					motor[2] -= 5;
			} else if (x < 5) { // should be < -5
				if (motor[2] <= 250)
					motor[2] += 5;
				if (motor[0] >= 5)
					motor[0] -= 5;
			}
			if (y > 15) { // should be > 5
				if (motor[1] <= 250)
					motor[1] += 5;
				if (motor[3] >= 5)
					motor[3] -= 5;
			} else if (y < 5) { // should be < -5
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
      break;
    case mcontrol_avoid:
			/*if (range[0] < MAX_RANGE && range[2] < MAX_RANGE
					&& range[1] < MAX_RANGE && range[3] < MAX_RANGE) {
				killed = 1;
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
				killed = 1;
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
      break;
    case mcontrol_killed:
      break;
    default:
      break;
  }

  switch(state) {
    case mcontrol_init:
      state = mcontrol_hover; // mcontrol_takeoff;
      break;
    case mcontrol_takeoff:
      if (killed) {
        state = mcontrol_killed;
      } else if (z > 5 || z < -5) {
        state = mcontrol_hover;
      }
      break;
    case mcontrol_hover:
      if (killed) {
        state = mcontrol_killed;
      } else if (obstacle) {
        state = mcontrol_avoid;
      } else {
        state = mcontrol_hover;
      }
      break;
    case mcontrol_avoid:
      if (killed) {
        state = mcontrol_killed;
      } else if (!obstacle) {
        state = mcontrol_hover;
      } else {
        state = mcontrol_avoid;
      }
      break;
    case mcontrol_killed:
      state = mcontrol_killed;
      break;
    default:
      state = mcontrol_init;
      break;

  }

  /* SET MOTOR SPEED */
  if (killed)
    for (i = 0; i < NUM_MOTOR; i++)
      motor[i] = 0;
  for (i = 0; i < NUM_MOTOR; i++)
    PWM_set(i, motor[i]);

  return state;
}


int main(void) {
	DDRA=0x00; PORTA=0xFF; // PORTA = Accelerometer input
	DDRB=0xFF; PORTB=0x00; // PORTB = Motor output
	DDRC=0x00; PORTC=0xFF; // PORTC = Sonar Input
	DDRD=0xFF; PORTD=0x00; // PORTD = Sonar/Motor output
	/* DEBUGGING */
	//DDRC=0xFF; PORTC=0x00; // PORTC = LCD output

  uint32_t accel_tick_calc = 100;
  uint32_t sonar_tick_calc = 100;
  uint32_t mcontrol_tick_calc = 100;

  uint32_t tmpGCD = 1;
  tmpGCD = findGCD(accel_tick_calc, sonar_tick_calc);
  tmpGCD = findGCD(tmpGCD, mcontrol_tick_calc);
  uint32_t GCD = tmpGCD;
  uint32_t accel_tick_period = accel_tick_calc/GCD;
  uint32_t sonar_tick_period = sonar_tick_calc/GCD;
  uint32_t mcontrol_tick_period = mcontrol_tick_calc/GCD;

  static task accel_task, sonar_task, mcontrol_task;
  task * tasks[] = { &accel_task, &sonar_task, &mcontrol_task };
  const uint8_t numTasks = sizeof(tasks)/sizeof(task*);

  accel_task.state = -1;
  accel_task.period = accel_tick_period;
  accel_task.elapsedTime = accel_tick_period;
  accel_task.TickFct = &accel_tick;

  sonar_task.state = -1;
  sonar_task.period = sonar_tick_period;
  sonar_task.elapsedTime = sonar_tick_period;
  sonar_task.TickFct = &sonar_tick;

  mcontrol_task.state = -1;
  mcontrol_task.period = mcontrol_tick_period;
  mcontrol_task.elapsedTime = mcontrol_tick_period;
  mcontrol_task.TickFct = &mcontrol_tick;

  TimerSet(GCD);
  TimerOn();

	ADC_init();
	PWM_init();
	/* DEBUGGING */
	//LCD_init();
	
  uint8_t i;
	while(1) {
    for (i = 0; i < numTasks; i++) {
      if (tasks[i]->elapsedTime >= tasks[i]->period) {
        tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
        tasks[i]->elapsedTime = 0;
      }
      tasks[i]->elapsedTime += 1;
    }
    while(!TimerFlag)
      ;
    TimerFlag = 0;
	}

	return(0);
}
