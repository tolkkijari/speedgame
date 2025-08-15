all: directory compile objcopy flash

directory:
	mkdir -p build

compile:
	avr-gcc -DF_CPU=1000000 -mmcu=atmega168 -Wall -Os -o build/speedgame.elf *.c

objcopy:
	avr-objcopy -j .text -j .data -O ihex build/speedgame.elf build/speedgame.hex
	
flash:
	avrdude -c um232h -p m168 -U flash:w:build/speedgame.hex