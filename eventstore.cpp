#include "eventstore.h"

EventStore::EventStore() {
  // Initialize object pool
  
  // mmap_info = 

  object_pool = new ObjectPool<Order>(MAX_OPEN_ORDERS, "pool_name3", "free_space_name3");

  mmap_info = init_mmap(name, shared_mem_size);
  event_store_buf = new (mmap_info->location) std::unordered_map<SEQUENCE_ID, Order*>;
 
  sequence = 0;
}

int EventStore::mmap_size() {
  return shared_mem_size;
}

EventStore::~EventStore() {
  mark_mmap_for_deletion(name, mmap_size());
}


SEQUENCE_ID EventStore::newEvent(SIDE side, PRICE limitPrice, char clientId, int quantity) {
  Order* order = object_pool->allocate();
  order->clientId = clientId;
  order->side = side;
  order->limitPrice = limitPrice;
  order->quantity = quantity;

  sequence++;
  order->id = sequence;

  event_store_buf->emplace(sequence, order);

  return sequence;
}

void EventStore::remove(SEQUENCE_ID id) {
  event_store_buf->erase(id);
}

Order * EventStore::get(SEQUENCE_ID id) {
  return event_store_buf->at(id);
}

size_t EventStore::size() {
  return event_store_buf->size();
}
