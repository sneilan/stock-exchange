#ifndef mmap_h
#define mmap_h

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>
#include <unordered_map>
#include <sstream>

struct MMapMeta {
  int fd;
  void *location;
  const char *name;
  int size;
};

MMapMeta *init_mmap(const char *name, int size);
MMapMeta *open_mmap(const char *name, int size);
void delete_mmap(MMapMeta *info);
void close_mmap(MMapMeta *info);

#endif
