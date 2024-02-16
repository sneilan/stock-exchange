#include "eventstore.h"

EventStore::EventStore(MMapObjectPool<Order> *order_pool) {
  sequence = 0;
  this->order_pool = order_pool;
}

EventStore::~EventStore() { }

SEQUENCE_ID EventStore::newEvent(SIDE side, PRICE limitPrice, int clientId,
                                 int quantity) {
  Order *order = order_pool->allocate();
  order->clientId = clientId;
  order->side = side;
  order->limitPrice = limitPrice;
  order->quantity = quantity;

  order->id = sequence;
  sequence++;

  ORDER_MMAP_OFFSET offset = order_pool->pointer_to_offset(order);

  event_store.emplace(sequence, offset);

  return sequence;
}

ORDER_MMAP_OFFSET EventStore::getOffset(SEQUENCE_ID id) {
  return event_store.at(id);
}

void EventStore::remove(SEQUENCE_ID id) { event_store.erase(id); }

Order *EventStore::get(SEQUENCE_ID id) {
  ORDER_MMAP_OFFSET offset = event_store.at(id);
  return order_pool->offset_to_pointer(offset);
}

size_t EventStore::size() { return event_store.size(); }
