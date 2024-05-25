#!/usr/bin/env sh

# This script create a boot image for this kernel.

SCRIPT_DIR="$( cd -- "$( dirname -- "$0" )" > /dev/null && pwd )"

PROJECT_PATH="$SCRIPT_DIR/.."
BUILD_DIR="$1"
TARGET_FILE="$2"

if [ "$BUILD_DIR" = "" ]; then
    echo "Unknown build dir"
    exit 1
fi

if [ "$TARGET_FILE" = "" ]; then
    echo "No target file"
    exit 1
fi

RASPI_FIRMWARE="$BUILD_DIR/firmware"

CONFIG_FILE="$PROJECT_PATH/kernel/boot/config.txt"
KERNEL_FILE="$BUILD_DIR/kernel/kernel8.img"
RAM_FS_FILE="$BUILD_DIR/binuser/fs.img"

#rm -rf "$RASPI_FIRMWARE"
git clone -n --depth=1 --filter=tree:0 -- https://github.com/raspberrypi/firmware.git "$RASPI_FIRMWARE"
cd "$RASPI_FIRMWARE" || exit 1
git sparse-checkout set --no-cone boot
git checkout
rm -v ./boot/kernel*.img
cd ..

dd if=/dev/zero bs=1M count=100 of="$TARGET_FILE"
mformat -i "$TARGET_FILE" -F
mcopy -s -i "$TARGET_FILE" "$RASPI_FIRMWARE/boot/"* ::
mcopy -i "$TARGET_FILE" "$KERNEL_FILE" ::
mcopy -i "$TARGET_FILE" "$CONFIG_FILE" ::
mcopy -i "$TARGET_FILE" "$RAM_FS_FILE" ::
mdir -i "$TARGET_FILE"
