#include "eventstore.h"

EventStore::EventStore() {
    const char * name = "/eventstore_buff";

    int sharedMemFd = shm_open(name, O_CREAT | O_RDWR, 0666);
    //size = sizeof(std::unordered_map<SEQUENCE_ID, Event>);

    ftruncate(sharedMemFd, shared_mem_size);

    void * sharedMemPointer = mmap(nullptr, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemFd, 0);
    if (sharedMemPointer == MAP_FAILED) {
        throw std::runtime_error("could not initialize /eventstore_buff mmap");
    }

    memset(sharedMemPointer, 0, shared_mem_size);
    // @TODO this is a memory leak.
    eventStoreBuf = new (sharedMemPointer) std::unordered_map<SEQUENCE_ID, Order>;

    // @TODO Handle errors returned by ftruncate.
    // @TODO do we need this if we are not operating on a file?
    ftruncate(sharedMemFd, sizeof(Order) * EVENTSTORE_BUFLEN);

    sequence = 0;
}

EventStore::~EventStore() {
// Unmap the memory and close the file
  if (munmap(sharedMemPointer, shared_mem_size) == -1) {
    perror("munmap");
  }

  close(sharedMemFd);
}

SEQUENCE_ID EventStore::newEvent(SIDE side, PRICE limitPrice, char clientId, int quantity) {
    Order order;
    order.clientId = clientId;
    order.side = side;
    order.limitPrice = limitPrice;
    order.quantity = quantity;

    sequence++;
    order.id = sequence;

    eventStoreBuf->insert(std::make_pair(sequence, order));

    return sequence;
}

Order * EventStore::get(SEQUENCE_ID id) {
    return &eventStoreBuf->at(id);
}

size_t EventStore::size() {
    return eventStoreBuf->size();
}