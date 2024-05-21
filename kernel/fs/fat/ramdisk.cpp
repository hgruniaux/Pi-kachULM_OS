#include "ff.h" /* Obtains integer types */

#include "diskio.h" /* Declarations of disk functions */

#include <libk/string.hpp>
#include "memory/mem_alloc.hpp"

// A sector takes 512 bytes.
constexpr LBA_t RAM_SECTOR_COUNT = (1 << 26) / FF_MIN_SS;  // 68 mb of RAM
static uint8_t* ramdisk_buffer = nullptr;

extern "C" {
DSTATUS disk_status(BYTE drive) {
  switch (drive) {
    case 0:
      return 0;
    default:
      return STA_NOINIT;
  }
}

DSTATUS disk_initialize(BYTE drive) {
  switch (drive) {
    case 0:
      ramdisk_buffer = (uint8_t*)kmalloc(RAM_SECTOR_COUNT * FF_MIN_SS, alignof(max_align_t));
      if (ramdisk_buffer == nullptr)
        return STA_NOINIT;
      return 0;
    default:
      return STA_NOINIT;
  }
}

DRESULT disk_read(BYTE drive, BYTE* buff, LBA_t sector, UINT count) {
  if (drive != 0 || ramdisk_buffer == nullptr)
    return RES_NOTRDY;

  libk::memcpy(buff, &ramdisk_buffer[sector * FF_MIN_SS], (size_t)count * FF_MIN_SS);
  return RES_OK;
}

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE drive, const BYTE* buff, LBA_t sector, UINT count) {
  if (drive != 0 || ramdisk_buffer == nullptr)
    return RES_NOTRDY;

  libk::memcpy(&ramdisk_buffer[sector * FF_MIN_SS], buff, (size_t)count * FF_MIN_SS);
  return RES_OK;
}

#endif

DRESULT disk_ioctl(BYTE drive, BYTE cmd, void* buff) {
  if (drive != 0 || ramdisk_buffer == nullptr)
    return RES_NOTRDY;

  switch (cmd) {
    case CTRL_SYNC:
      return RES_OK;  // nothing to sync
    case GET_SECTOR_COUNT:
      *(LBA_t*)buff = RAM_SECTOR_COUNT;
      break;
    case GET_SECTOR_SIZE:
      *(WORD*)buff = FF_MIN_SS;
      break;
    case GET_BLOCK_SIZE:
      *(DWORD*)buff = 1;
      return RES_OK;
    case CTRL_TRIM:
      return RES_OK;  // not a flash memory device (not supported nor needed)
    default:
      return RES_PARERR;
  }

  return RES_PARERR;
}
}
