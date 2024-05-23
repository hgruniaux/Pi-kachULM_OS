#pragma once

#include <sys/file.h>
#include <cstddef>
#include "fat/ff.h"

class Dir {
 public:
  bool read(sys_file_info_t* file_info);

 private:
  friend class FileSystem;
  DIR m_handle;
};  // class Dir
