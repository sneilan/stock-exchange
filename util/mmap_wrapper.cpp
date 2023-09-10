#include "mmap_wrapper.h"

MMap_Info *init_mmap(const char *name, int size) {
  int fd = shm_open(name, O_CREAT | O_RDWR, 0777);

  if (fd == -1) {
    throw std::runtime_error(
        "Could not open file descriptor to mmap in controller.");
  }

  if (ftruncate(fd, size) == -1) {
    throw std::runtime_error("Could not resize mmap in controller");
  }

  void *location =
      mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (location == MAP_FAILED) {
    throw std::runtime_error("Could not map mmap region to controller.");
  }

  new (location) char[size];

  memset(location, 0, size);

  // This is allocated w/o a destructor as it is assumed
  // program will instantiate an mmap once.
  MMap_Info *info = new MMap_Info();

  info->location = location;
  info->fd = fd;
  info->name = name;
  info->size = size;

  return info;
};

void delete_mmap(MMap_Info *info) {
  munmap(info->location, info->size);
  shm_unlink(info->name);
}

MMap_Info *open_mmap(const char *name, int size) {
  int fd = shm_open(name, O_RDWR, 0777);

  if (fd == -1) {
    throw std::runtime_error(
        "Could not open file descriptor to mmap in client.");
  }

  void *location =
      mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (location == MAP_FAILED) {
    throw std::runtime_error("Could not map mmap region to client.");
  }

  // This is allocated w/o a destructor as it is assumed
  // program will instantiate an mmap once.
  MMap_Info *info = new MMap_Info();
  info->location = location;
  info->fd = fd;
  info->name = name;
  info->size = size;

  return info;
}

void close_mmap(MMap_Info *info) {
  munmap(info->location, info->size);
  close(info->fd);
}
