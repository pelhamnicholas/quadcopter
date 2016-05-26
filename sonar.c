// Assumes all sonar sensors are on PORTD
void sonar_init(uint8_t trigger, uint8_t echo) {
  DDRD |= (1 << trigger); PORTD &= ~(1 << trigger);
  DDRD &= ~(1 << echo); PORTD |= (1 << echo);
}

// Assumes all rangefinders are on PORTD
// Taken from getPulse() in Arduino
uint16_t get_range_ms(uint8_t trigger, uint8_t echo) {
  uint16_t maxloops = microsecondsToClockCycles(50) / 16;
  uint16_t numloops = 0;
  uint16_t width = 0;

  // Send out pulse
  PORTD &= ~(1 << trigger);
  _delay_us(2);
  PORTD |= (1 << trigger);
  _delay_us(5);
  PORTD &= ~(1 << trigger);

  // Get echo pulse
  // Expects initial high value which will be ignored
  while (PIND & (1 << echo)) {
    if (numloops++ == maxloops)
      return -1;
  }
  while (PIND & (1 << echo)) {
    if (numloops++ == maxloops)
      return -1;
  }
  while (PIND & (1 << echo)) {
    if (numloops++ == maxloops)
      return -1;
    width++;
  }
    
  return clockCyclesToMicroseconds(width * 21 + 16);
}
