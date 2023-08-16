#include "eventstore.h"

EventStore::EventStore() {
  object_pool = new MMapObjectPool<Order>(MAX_OPEN_ORDERS, "pool_name3", true);

  mmap_info = init_mmap(name, shared_mem_size);
  event_store_buf = new (mmap_info->location) std::unordered_map<SEQUENCE_ID, Order*>;
 
  sequence = 0;
}

int EventStore::mmap_size() {
  return shared_mem_size;
}

EventStore::~EventStore() {
  delete_mmap(mmap_info);
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
