#include "task.h"

uint32_t findGCD(uint32_t a, uint32_t b) {
  uint32_t c;
  while(1) {
    c = a%b;
    if (c == 0) {return b;}
    a = b;
    b = c;
  }
  return 0;
}
