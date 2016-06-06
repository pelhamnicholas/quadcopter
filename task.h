#ifndef _TASK_H_
#define _TASK_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

typedef struct _task {
  int8_t state;
  uint32_t period;
  uint32_t elapsedTime;
  int (*TickFct)(int);
} task;

uint32_t findGCD(uint32_t, uint32_t);

#endif
