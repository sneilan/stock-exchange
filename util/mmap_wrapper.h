#ifndef mmap_h
#define mmap_h

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>
#include <unordered_map>
#include <spdlog/spdlog.h>

struct MMap_Info {
  int fd;
  void *location;
  const char *name;
  int size;
};

MMap_Info *init_mmap(const char *name, int size);
MMap_Info *open_mmap(const char *name, int size);
void delete_mmap(MMap_Info *info);
void close_mmap(MMap_Info *info);

#endif
