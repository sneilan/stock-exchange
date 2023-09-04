#ifndef _eventstore_h
#define _eventstore_h

#include "./util/types.h"
#include "util/mmap_wrapper.h"
#include "util/object_pool.h"
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>
#include <unordered_map>

#define MAX_OPEN_ORDERS 10000

class EventStore {
public:
  EventStore();
  ~EventStore();
  SEQUENCE_ID newEvent(SIDE side, PRICE limitPrice, int clientId, int quantity);
  Order *get(SEQUENCE_ID id);
  void remove(SEQUENCE_ID id);
  size_t size();

private:
  // Location of shared memory block where we store orders.
  MMap_Info *mmap_info;
  const char *name = "/eventstore_buf";

  SEQUENCE_ID sequence;
  std::unordered_map<SEQUENCE_ID, ORDER_MMAP_OFFSET> event_store;
  MMapObjectPool<Order> *object_pool;
};

#endif
