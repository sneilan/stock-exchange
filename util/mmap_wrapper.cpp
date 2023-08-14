#include "mmap_wrapper.h"

MMap_Info * init_mmap(const char * name, int size) {
  int fd = shm_open(name, O_CREAT | O_RDWR, 0666);

  if (ftruncate(fd, size) == -1) {
    throw std::runtime_error("Could not resize mmap");
  }

  void * location = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (location == MAP_FAILED) {
    throw std::runtime_error("could not initialize mmap");
  }

  memset((wchar_t *)location, 0, size);

  MMap_Info * info = new MMap_Info();
  info->location = location;
  info->fd = fd;

  return info;
};

MMap_Info * open_mmap(const char * name, int size) {
  int fd = shm_open(name, O_CREAT | O_RDWR, 0666);

  void * location = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (location == MAP_FAILED) {
    throw std::runtime_error("could not initialize mmap");
  }

  MMap_Info * info = new MMap_Info();
  info->location = location;
  info->fd = fd;

  return info;
}

void mark_mmap_for_deletion(const char * name, void * location, int size) {
  munmap(location, size);
  shm_unlink(name);
}

void close_mmap(void * location, int size, int fd) {
  munmap(location, size);
  close(fd);
}
