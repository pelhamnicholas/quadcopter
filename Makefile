# Minimal Makefile for avr-gcc
PROJECT := quadcopter
SOURCES := main.c io.c
CC := avr-gcc
OBJCOPY := avr-objcopy
MMCU := atmega1284
OPT := s
CFLAGS := -mmcu=$(MMCU) -Wall -Werror -O$(OPT)

$(PROJECT).hex: $(PROJECT).out
	$(OBJCOPY) -j .text -O ihex $(PROJECT).out $(PROJECT).hex

$(PROJECT).out: $(SOURCE)
	$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)

program: $(PROJECT).hex
	avrdude -P /dev/usb/hiddev0 -p m1284 -c jtagmkII -e -U flash:w:$(PROJECT).hex

clean:
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex
