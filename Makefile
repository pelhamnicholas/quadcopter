# Minimal Makefile for avr-gcc
PROJECT := quadcopter
SOURCES := main.c io.c sonar.c accelerometer.c
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
	avrdude -p m1284 -c atmelice_pdi -e -U flash:w:$(PROJECT).hex
#-P /dev/usb/hiddev0

clean:
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex
