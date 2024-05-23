#!/usr/bin/env sh

# To create the ramfs image used by Pi-kachULM_OS, please call this script with the name of the file to produce:
# ./create-fs.sh `target ramfs file path`

TARGET_RAM_FS_NAME="$1"
TARGET_RAM_FS_SIZE=10 #Mio

if [ "$TARGET_RAM_FS_NAME" = "" ]; then
    echo "Please specify target file."
    exit 1
fi

SCRIPT_DIR="$( cd -- "$( dirname -- "$0" )" > /dev/null && pwd )"
RAM_FS_DIR="$SCRIPT_DIR/../fs"

DD_EXEC=$(command -v dd)
if [ "$DD_EXEC" = "" ]; then
     echo "'dd' not found. Please install 'coreutils'. (Wtf ?)"
     exit 2
fi

MFORMAT_EXEC=$(command -v mformat)
if [ "$MFORMAT_EXEC" = "" ]; then
     echo "'mformat' not found. Please install 'mtools'."
     exit 2
fi

MCOPY_EXEC=$(command -v mcopy)
if [ "$MCOPY_EXEC" = "" ]; then
     echo "'mcopy' not found. Please install 'mtools'."
     exit 2
fi

MDIR_EXEC=$(command -v mdir)
if [ "$MDIR_EXEC" = "" ]; then
     echo "'mdir' not found. Please install 'mtools'."
     exit 2
fi


# Create the image
$DD_EXEC if=/dev/zero bs=1M count="$TARGET_RAM_FS_SIZE" of="$TARGET_RAM_FS_NAME" > /dev/null 2>&1

# Create the FAT filesystem in it
$MFORMAT_EXEC -i "$TARGET_RAM_FS_NAME" -M 512

# Populate it
$MCOPY_EXEC -s -i "$TARGET_RAM_FS_NAME" "$RAM_FS_DIR"/* ::

# Check
$MDIR_EXEC -s -i "$TARGET_RAM_FS_NAME" ::
