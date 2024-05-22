#include "dir.hpp"
#include <libk/assert.hpp>
#include <libk/string.hpp>

bool Dir::read(sys_file_info_t* file_info) {
  KASSERT(file_info != nullptr);

  FILINFO filinfo;
  if (f_readdir(&m_handle, &filinfo) != FR_OK || filinfo.fname[0] == 0)
    return false;

  libk::memcpy(file_info->name, filinfo.fname, sizeof(filinfo.fname));
  file_info->is_dir = (filinfo.fattrib & AM_DIR) != 0;
  return true;
}
