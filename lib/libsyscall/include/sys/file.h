#ifndef __PIKAOS_LIBC_SYS_FILE_H__
#define __PIKAOS_LIBC_SYS_FILE_H__

#include "__types.h"
#include "__utils.h"

__SYS_EXTERN_C_BEGIN

typedef struct sys_file_t sys_file_t;

typedef struct sys_dir_t sys_dir_t;

typedef struct sys_file_info_t {
  char name[256];
  sys_bool_t is_dir;
} sys_file_info_t;

typedef enum sys_file_mode_t {
  SYS_FM_READ = 0x1,
  SYS_FM_WRITE = 0x2,
} sys_file_mode_t;

sys_file_t* sys_open_file(const char* path, sys_file_mode_t mode);
void sys_close_file(sys_file_t* file);

sys_error_t sys_file_read(sys_file_t* file, void* buffer, size_t bytes_to_read, size_t* read_bytes);
size_t sys_get_file_size(sys_file_t* file);

sys_dir_t* sys_open_dir(const char* path);
void sys_close_dir(sys_dir_t* dir);
sys_error_t sys_read_dir(sys_dir_t* dir, sys_file_info_t* info);

__SYS_EXTERN_C_END

#endif  // !__PIKAOS_LIBC_SYS_FILE_H__
