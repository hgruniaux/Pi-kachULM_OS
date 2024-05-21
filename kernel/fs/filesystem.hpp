#pragma once

#include "file.hpp"

class FileSystem {
 public:
  using FileHandle = void*;

  static FileSystem& get();

  void init();

  File* open(const char* path, int flags);
  void close(File* handle);
};  // class FileSystem
