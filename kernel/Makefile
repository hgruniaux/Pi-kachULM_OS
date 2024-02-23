CFILES = $(wildcard *.c)
CXXFILES = $(wildcard *.cxx)
COFILES = $(CFILES:.c=.o)
CXXOFILES = $(CXXFILES:.cxx=.o)
CFLAGS = -g -Wall -Wextra -fno-omit-frame-pointer -ffreestanding -nostdlib -nostartfiles
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++20
GCCPREFIX ?= aarch64-linux-gnu-

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

all: kernel8.img

boot.o: boot.S
	$(GCCPREFIX)gcc $(DEPFLAGS) $(CFLAGS) -c boot.S -o boot.o

$(COFILES): %.o: %.c $(DEPDIR)/%.d | $(DEPDIR)
	$(GCCPREFIX)gcc $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(CXXOFILES): %.o: %.cxx $(DEPDIR)/%.d | $(DEPDIR)
	$(GCCPREFIX)g++ $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@

kernel8.elf: boot.o $(COFILES) $(CXXOFILES)
	$(GCCPREFIX)ld -nostdlib boot.o $(COFILES) $(CXXOFILES) -T link.ld -o kernel8.elf

kernel8.img: kernel8.elf
	$(GCCPREFIX)objcopy -O binary kernel8.elf kernel8.img

clean:
	rm -f $(wildcard *.o)
	rm -f kernel8.img
	rm -f kernel8.elf

$(DEPDIR): ; @mkdir -p $@

DEPCFILES := $(CFILES:%.c=$(DEPDIR)/%.d)
$(DEPCFILES):

include $(wildcard $(DEPCFILES))

DEPCXXFILES := $(CXXFILES:%.cxx=$(DEPDIR)/%.d)
$(DEPCXXFILES):

include $(wildcard $(DEPCXXFILES))

.PHONY: all clean
