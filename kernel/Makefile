CFILES = $(wildcard *.c)
CXXFILES = $(wildcard *.cxx)
COFILES = $(CFILES:.c=.o)
CXXOFILES = $(CXXFILES:.cxx=.o)
CFLAGS = -Wall -Wextra -O2 -g -ffreestanding -nostdlib -nostartfiles
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++20
GCCPREFIX ?= aarch64-linux-gnu-

all: kernel8.img

boot.o: boot.S
	$(GCCPREFIX)gcc $(CFLAGS) -c boot.S -o boot.o

$(COFILES): %.o: %.c
	$(GCCPREFIX)gcc $(CFLAGS) -c $< -o $@

$(CXXOFILES): %.o: %.cxx
	$(GCCPREFIX)g++ $(CXXFLAGS) -c $< -o $@

kernel8.elf: boot.o $(COFILES) $(CXXOFILES)
	$(GCCPREFIX)ld -nostdlib boot.o $(COFILES) $(CXXOFILES) -T link.ld -o kernel8.elf

kernel8.img: kernel8.elf
	$(GCCPREFIX)objcopy -O binary kernel8.elf kernel8.img

clean:
	rm -f $(wildcard *.o)
	rm -f kernel8.img
	rm -f kernel8.elf

.PHONY: all clean
