#include <avr/io.h>
#include <avr/interrupt.h>
#include "sonar.h"
#include <util/delay.h>

// Assumes all sonar sensors are on PORTD
void sonar_init(uint8_t trigger, uint8_t echo) {
  DDRD |= (1 << trigger); PORTD &= ~(1 << trigger);
  // Not setting input PORTD pins to high avoids some errors
  DDRC &= ~(1 << echo); PORTC |= (1 << echo);
}

// Assumes all rangefinders are on PORTD
// Taken from pulseIn() in Arduino
uint16_t get_range_us(uint8_t trigger, uint8_t echo) {
  uint16_t maxloops = millisecondsToClockCycles(1000);// / 16;
  uint16_t numloops = 0;
  uint16_t width = 0;

  // Send out pulse
  PORTD &= ~(1 << trigger);
  _delay_us(2);
  PORTD |= (1 << trigger);
  _delay_us(15); // Maybe use a higher value?
  PORTD &= ~(1 << trigger);

  // Get echo pulse
  // Expects initial high value which will be ignored
  while (PINC & (1 << echo)) {
    if (numloops++ == maxloops)
      return -1;
  }
  while (!(PINC & (1 << echo))) {
    if (numloops++ == maxloops)
      return -1;
  }
  while (PINC & (1 << echo)) {
    if (numloops++ == maxloops)
      return -1;
    width++;
  }
    
  return clockCyclesToMicroseconds(width * 21 + 16);
}
