# Minimal Makefile for avr-gcc
PROJECT := quadcopter
SOURCES := main.c io.c
CC := avr-gcc
OBJCOPY := avr-objcopy
MMCU := atmega1284
OPT := s
CFLAGS := -mmcu=$(MMCU) -Wall -O$(OPT)
  
$(PROJECT).hex: $(PROJECT).out
	$(OBJCOPY) -j .text -O ihex $(PROJECT).out $(PROJECT).hex

$(PROJECT).out: $(SOURCE)
	$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)

program: $(PROJECT).hex
	avrdude -P /dev/usb/hiddev0 -p m1284p -c dapa -e -U flash:w:$(PROJECT).hex

clean:
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex
