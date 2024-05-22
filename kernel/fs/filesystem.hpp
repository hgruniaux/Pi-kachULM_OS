#pragma once

#include "dir.hpp"
#include "file.hpp"

class FileSystem {
 public:
  static FileSystem& get();

  void init();

  File* open(const char* path, int flags);
  void close(File* handle);

  Dir* open_dir(const char* path);
  void close_dir(Dir* handle);
};  // class FileSystem
