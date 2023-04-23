#include "eventstore.h"

EventStore::EventStore() {
    const char * name = "/eventstore_buff";
    fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    //size = sizeof(std::unordered_map<SEQUENCE_ID, Event>);
    shared_mem_size = 100000;

    ftruncate(fd, shared_mem_size);

    ptr = mmap(nullptr, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        throw std::runtime_error("could not initialize /eventstore_buff mmap");
    }

    eventStoreBuf = new (ptr) std::unordered_map<SEQUENCE_ID, Event>;

    // @TODO Handle errors returned by ftruncate.
    // @TODO do we need this if we are not operating on a file?
    ftruncate(fd, sizeof(Event) * EVENTSTORE_BUFLEN);

    sequence = 0;
}

EventStore::~EventStore() {
// Unmap the memory and close the file
  if (munmap(ptr, shared_mem_size) == -1) {
    perror("munmap");
  }

  close(fd);
}

SEQUENCE_ID EventStore::newEvent(char side, float limitPrice, char clientId) {
    Event event;
    event.clientId = clientId;
    event.side = side;
    event.limitPrice = limitPrice;

    sequence++;
    event.sequence = sequence;

    eventStoreBuf->insert(std::make_pair(sequence, event));

    return sequence;
}

Event EventStore::get(SEQUENCE_ID id) {
    return eventStoreBuf->at(id);
}

size_t EventStore::size() {
    return eventStoreBuf->size();
}