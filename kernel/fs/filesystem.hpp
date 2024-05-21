#pragma once

class FileSystem {
 public:
  using FileHandle = void*;

  static FileSystem& get();

  void init();

  FileHandle open(const char* path, int flags);
  void close(FileHandle handle);
};  // class FileSystem
