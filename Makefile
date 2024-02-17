all: kernel

kernel:
	$(MAKE) -C kernel

clean:
	$(MAKE) -C kernel clean

.PHONY: all kernel clean
