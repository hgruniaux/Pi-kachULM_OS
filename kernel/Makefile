CFILES = $(wildcard *.c)
CXXFILES = $(wildcard *.cxx)
OFILES = $(CFILES:.c=.o)
CFLAGS = -Wall -Wextra -O2 -ffreestanding -nostdlib -nostartfiles
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++20
GCCPREFIX ?= aarch64-linux-gnu-

all: clean kernel8.img

boot.o: boot.S
	$(GCCPREFIX)gcc $(GCCFLAGS) -c boot.S -o boot.o

%.o: %.c
	$(GCCPREFIX)gcc $(CFLAGS) -c $< -o $@

%.o: %.cxx
	$(GCCPREFIX)g++ $(CXXFLAGS) -c $< -o $@

kernel8.elf: boot.o $(OFILES)
	$(GCCPREFIX)ld -nostdlib boot.o $(OFILES) -T link.ld -o kernel8.elf

kernel8.img: kernel8.elf
	$(GCCPREFIX)objcopy -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf *.o *.img > /dev/null 2> /dev/null || true
