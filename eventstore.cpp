#include "eventstore.h"
#include "util/object_pool.h"

EventStore::EventStore() {
  object_pool = new MMapObjectPool<Order>(MAX_OPEN_ORDERS, name, IS_CONTROLLER);
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

  order->id = sequence;
  sequence++;

  // @TODO implement hash map so we can index orders by id.
  // Then when traders want to update / cancel an order, they can give us the id
  // and we can map it to an order.
  return sequence;
}

void EventStore::remove(SEQUENCE_ID id) {

  // event_store_buf->erase(id);
}

Order * EventStore::get(SEQUENCE_ID id) {
  return 0;
  // return event_store_buf->at(id);
}

size_t EventStore::size() {
  return 0;
  // return event_store_buf->size();
}
