#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"


extern volatile unsigned char TimerFlag;    // TimerISR() sets this to 1.
                                            //C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0.
                                // Default 1 ms.
unsigned long _avr_timer_cntcurr = 0;   // Current internal count of 1 ms ticks.

void TimerOn() {
	TimerFlag = 0;
	
    // AVR timer/counter controller register TCCR1
    TCCR1B = 0x0B;      // bit 3 = 0: CTC mode (clear timer on compare)
                        // bit2bit1bit0=011: pre-scaler /64
                        // 00001011: 0x0B
                        // So, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
                        // Thus, TCNT1 register will count at 125,000 ticks/s

    // AVR output compare register OCR1A
    OCR1A = 125;        // Timer interrupt will be generated when TCNT==OCR1A
                        // We want a 1 ms tick.

    // AVR timer interrupt mask register
    TIMSK1 = 0x02;

    // Initialize AVR counter
    TCNT1 = 0;

    _avr_timer_cntcurr = _avr_timer_M;
    // TimerISR will be called every _avr_timer_cntcurr milliseconds

    // Enable global interrupts
    SREG |= 0x80;
}

void TimerOff() {
    TCCR1B = 0x00;
}

void TimerISR() {
    TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
    _avr_timer_cntcurr--;
    if(_avr_timer_cntcurr == 0) {
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}

void TimerSet(unsigned long M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}
