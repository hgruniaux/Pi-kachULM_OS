CFILES = $(wildcard *.c)
CXXFILES = $(wildcard *.cxx)
OFILES = $(CFILES:.c=.o)
CFLAGS = -Wall -Wextra -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++20
GCCPREFIX ?= aarch64-none-elf-

all: clean kernel.img

boot.o: boot.S
	$(GCCPREFIX)gcc $(GCCFLAGS) -c boot.S -o boot.o

%.o: %.c
	$(GCCPREFIX)gcc $(CFLAGS) -c $< -o $@

%.o: %.cxx
	$(GCCPREFIX)g++ $(CXXFLAGS) -c $< -o $@

kernel.img: boot.o $(OFILES)
	$(GCCPREFIX)ld -nostdlib boot.o $(OFILES) -T link.ld -o kernel.elf
	$(GCCPREFIX)objcopy -O binary kernel.elf kernel.img

clean:
	rm kernel.elf *.o *.img > /dev/null 2> /dev/null || true
