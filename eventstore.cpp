#include "eventstore.h"
#include "util/object_pool.h"
#include "util/types.h"

// @TODO Check if this is an extern reference.
const char *eventstore_buf_name = "/eventstore_buf";

EventStore::EventStore(MMapObjectPool<Order> *object_pool) {
  sequence = 0;
  this->object_pool = object_pool;
}

// @TODO I want highest level managing mmaps.
EventStore::~EventStore() { delete_mmap(mmap_info); }

SEQUENCE_ID EventStore::newEvent(SIDE side, PRICE limitPrice, int clientId,
                                 int quantity) {
  Order *order = object_pool->allocate();
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

ORDER_MMAP_OFFSET EventStore::getOffset(SEQUENCE_ID id) {
  return event_store.at(id);
}

void EventStore::remove(SEQUENCE_ID id) { event_store.erase(id); }

Order *EventStore::get(SEQUENCE_ID id) {
  ORDER_MMAP_OFFSET offset = event_store.at(id);
  return object_pool->offset_to_pointer(offset);
}

size_t EventStore::size() { return event_store.size(); }
