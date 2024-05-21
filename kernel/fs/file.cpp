#include "file.hpp"

bool File::read(void* buffer, size_t bytes_to_read, size_t* read_bytes) {
  UINT read_bytes_bis;
  auto result = f_read(&m_handle, buffer, bytes_to_read, &read_bytes_bis);
  if (read_bytes != nullptr)
    *read_bytes = read_bytes_bis;
  return result == FR_OK;
}

bool File::write(const void* buffer, size_t bytes_to_write, size_t* wrote_bytes) {
#if !FF_FS_READONLY
  UINT wrote_bytes_bis;
  auto result = f_write(&m_handle, buffer, bytes_to_write, &wrote_bytes_bis);
  if (wrote_bytes != nullptr)
    *wrote_bytes = wrote_bytes_bis;
  return result == FR_OK;
#else
  return false;
#endif
}

bool File::seek(long long int offset) {
  return f_lseek(&m_handle, offset) == FR_OK;
}

bool File::truncate() {
#if !FF_FS_READONLY
  return f_truncate(&m_handle) == FR_OK;
#else
  return false;
#endif
}
