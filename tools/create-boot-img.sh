#!/usr/bin/env sh

# This script create a boot image for this kernel.

SCRIPT_DIR="$( cd -- "$( dirname -- "$0" )" > /dev/null && pwd )"

PROJECT_PATH="$SCRIPT_DIR/.."
RAMFS_BUILD_SCRIPT="$SCRIPT_DIR/create-fs.sh"
TARGET_FILE="$(readlink -f $1)"
TMP_DIR=$(mktemp -d)

RASPI_FIRMWARE="$TMP_DIR/firmware"
BUILD_DIR="$TMP_DIR/build"
KERNEL_BUILT_FILE="$TMP_DIR/build/kernel/kernel8.img"
CONFIG_FILE="$TMP_DIR/config.txt"
RAM_FS_FILE="$TMP_DIR/fs.img"


get_firmware() {
    git clone -n --depth=1 --filter=tree:0 -- https://github.com/raspberrypi/firmware.git "$RASPI_FIRMWARE"
    cd "$RASPI_FIRMWARE" || exit 1
    git sparse-checkout set --no-cone boot
    git checkout
    rm -v ./boot/kernel*.img
    cd ..
}

build_kernel() {
    cmake -S "$PROJECT_PATH" -B "$BUILD_DIR" --toolchain="$PROJECT_PATH/cmake/ClangToolchain.cmake" -DCMAKE_BUILD_TYPE=Release -DCLANG_TRIPLE=aarch64-linux-gnu -DCLANG_SUFFIX=-17
    make -s -C "$BUILD_DIR" kernel-img -j
}

create_config_txt() {
    echo "# Run in 64-bit mode
arm_64bit=1

# Load RamFs
ramfsfile=fs.img
ramfsaddr=0x18000000
" > "$CONFIG_FILE"
}

create_ramfs() {
    sh "$RAMFS_BUILD_SCRIPT" "$RAM_FS_FILE"
}

create_img () {
    dd if=/dev/zero bs=1M count=100 of="$TARGET_FILE"
    mformat -i "$TARGET_FILE" -F
    mcopy -s -i "$TARGET_FILE" "$RASPI_FIRMWARE/boot"/* ::
    mcopy -i "$TARGET_FILE" "$KERNEL_BUILT_FILE" ::
    mcopy -i "$TARGET_FILE" "$CONFIG_FILE" ::
    mcopy -i "$TARGET_FILE" "$RAM_FS_FILE" ::
    mdir -i "$TARGET_FILE"
}


build_kernel
create_config_txt
create_ramfs
get_firmware
create_img

rm -rf "$TMP_DIR"
