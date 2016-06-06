#ifndef _TIMER_H_
#define _TIMER_H_

volatile unsigned char TimerFlag;   // TimerISR() sets this to 1. 
                                    //C programmer should clear to 0.

void TimerOn();
void TimerOff();
void TimerISR();
void TimerSet(unsigned long M);

#endif
