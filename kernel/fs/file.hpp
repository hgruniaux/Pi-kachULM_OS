#pragma once

#include <cstddef>
#include "fat/ff.h"

class File {
 public:
  /** Returns the file size in bytes. */
  [[nodiscard]] size_t get_size() const { return f_size(&m_handle); }

  bool read(void* buffer, size_t bytes_to_read, size_t* read_bytes);
  bool write(const void* buffer, size_t bytes_to_write, size_t* wrote_bytes);
  bool seek(long long offset);
  size_t tell() const { return f_tell(&m_handle); }
  bool truncate();
  bool eof() const { return f_eof(&m_handle); }

 private:
  friend class FileSystem;
  FIL m_handle;
};  // class File
