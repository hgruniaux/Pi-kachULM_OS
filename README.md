# OS

## Building the kernel

To build the kernel you will need to install a aarch64 GCC compiler and `cmake`:

- On Ubuntu, something like that:
  ```shell
  sudo apt install gcc-aarch64-linux-gnu make cmake
  ```
  Try to install the latest available version of GCC.
- On macOS,
  ```shell
  brew install gcc make cmake
  ```
  should suffice if you are on a M1/M2 CPU. [To be tested]
- On Windows, good luck. See MinGW or search internet.

Then, you just need you execute in the project directory:

```shell
# Change aarch64-linux-gnu- by whatever is named GCC for aarch64 on your system.
cmake -S . -Bbuild -DGCC_PREFIX=aarch64-linux-gnu- --toolchain=cmake/GCCToolchain.cmake
make -j -C build
```

## Testing the kernel

You can either run the kernel on real Raspberry PI (at least version 3 required) hardware.

Or use QEMU. First, you need to install it:

- On ubuntu,
  ```shell
  sudo apt install qemu-system-arm
  ```
- On macOS,
  ```shell
  brew install qemu
  ```

Then to run the kernel, just type:

```shell
# You may need to change qemu-system-aarch64 by whatever is QEMU for aarch64 is named on your computer.
qemu-system-aarch64 -M raspi3b -serial stdio -kernel build/kernel/kernel8.elf
```
