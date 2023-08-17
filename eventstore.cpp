#include "eventstore.h"
#include "util/object_pool.h"

EventStore::EventStore() {
  object_pool = new MMapObjectPool<Order>(MAX_OPEN_ORDERS, name, IS_CONTROLLER);
  sequence = 0;
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

  ORDER_MMAP_OFFSET offset = object_pool->pointer_to_offset(order);

  event_store.emplace(sequence, offset);

  return sequence;
}

void EventStore::remove(SEQUENCE_ID id) {
  event_store.erase(id);
}

Order* EventStore::get(SEQUENCE_ID id) {
  ORDER_MMAP_OFFSET offset = event_store.at(id);
  return object_pool->offset_to_pointer(offset);
}

size_t EventStore::size() {
  return event_store.size();
}
