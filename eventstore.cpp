#include "eventstore.h"

EventStore::EventStore() {
    fd = shm_open("/eventstore_buff", O_CREAT | O_RDWR, 0666);
    size = sizeof(std::unordered_map<SEQUENCE_ID, Event>);
    // @TODO define ptr better.
    ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        throw std::runtime_error("mmap");
    }

    eventStoreBuf = new (ptr) std::unordered_map<SEQUENCE_ID, Event>;

    // @TODO Handle errors returned by ftruncate.
    // @TODO do we need this if we are not operating on a file?
    ftruncate(fd, sizeof(Event) * EVENTSTORE_BUFLEN);
}

EventStore::~EventStore() {
// Unmap the memory and close the file
  if (munmap(ptr, size) == -1) {
    perror("munmap");
  }

  close(fd);
}