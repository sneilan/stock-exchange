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

extern const char *eventstore_buf_name;

class EventStore {
public:
  EventStore(MMapObjectPool<Order> *order_pool);
  ~EventStore();
  SEQUENCE_ID newEvent(SIDE side, PRICE limitPrice, int clientId, int quantity);
  Order *get(SEQUENCE_ID id);
  ORDER_MMAP_OFFSET getOffset(SEQUENCE_ID id);
  void remove(SEQUENCE_ID id);
  size_t size();

private:
  SEQUENCE_ID sequence;
  std::unordered_map<SEQUENCE_ID, ORDER_MMAP_OFFSET> event_store;
  MMapObjectPool<Order> *order_pool;
};

#endif
