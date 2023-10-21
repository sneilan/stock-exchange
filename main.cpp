#include "eventstore.h"
#include "gateway/gateway.h"
#include "gateway/socket.h"
#include "order_book/order_book.h"
#include "util/types.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/version.h>
#include <sys/mman.h>
#include <unistd.h>

// @TODO should be able to understand flow of information throughout entire program in this file.
int main() {
  spdlog::set_level(spdlog::level::debug);
  // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  spdlog::set_pattern("%-5l %E %-16s%-4#%-21! %v");

  // @TODO provide better name for this.
  Producer<ORDER_MMAP_OFFSET> outgoingDisruptor(MAX_OUTGOING_MESSAGES,
                                                OUTGOING_MESSAGE_BUFFER);

  SPDLOG_INFO("Allocating EventStore mmap pool..");
  MMapObjectPool<Order> *object_pool = new MMapObjectPool<Order>(
      MAX_OPEN_ORDERS, eventstore_buf_name, IS_CONTROLLER);
  SPDLOG_INFO("Allocated EventStore mmap pool!");

  // @TODO Instantiate producer, consumer and mmap pool and pass those as parameters.
  Gateway *gateway = new Gateway();

  SPDLOG_INFO("Exchange starting");

  pid_t c_pid = fork();

  if (c_pid == -1) {
    SPDLOG_CRITICAL("fork");
    exit(EXIT_FAILURE);
  }

  if (c_pid > 0) {
    // Parent
    // Listens to new orders from clients and puts them into the mmap ring
    // buffer maintained by gateway.
    SPDLOG_INFO("Gateway starting");
    gateway->run();
  } else {
    // Child
    SPDLOG_INFO("Order engine starting");
    EventStore *eventStore = new EventStore(object_pool);
    SPDLOG_INFO("Created EventStore");
    OrderBook *orderBook = new OrderBook();
    SPDLOG_INFO("Created OrderBook");

    while (1) {
      // Constantly checking for new orders in the gateway ring buffer.
      NewOrderEvent *item = gateway->get();
      if (item == nullptr) {
        continue;
      }

      // Store the event in the event store
      SEQUENCE_ID id = eventStore->newEvent(item->side, item->limitPrice,
                                            item->clientId, item->quantity);
      SPDLOG_INFO("Sequence ID is now {} & size is now {}", id,
                  eventStore->size());

      // Get response here & spool information to new ring buffer
      Order *order = eventStore->get(id);
      ORDER_MMAP_OFFSET offset = eventStore->getOffset(id);
      SPDLOG_INFO("Grabbed order {}", order->id);
      std::list<Order *> updated_orders = orderBook->newOrder(order);

      // later on create a new disruptor for order recieved messages
      outgoingDisruptor.put(offset);
      // State of order is based on how many fills.
      SPDLOG_DEBUG("Order {} recieved message sent", order->id);

      SPDLOG_INFO("Order book volume is now {}", orderBook->getVolume());
      SPDLOG_INFO("Orders updated are size {}", updated_orders.size());

      for (Order *order : updated_orders) {
        // @TODO Stop using socket ids as client ids. Set up a map
        // between client ids and sockets. Also create a buffer to try
        // to send orders to clients that have disconnected.
        outgoingDisruptor.put(object_pool->pointer_to_offset(order));
        SPDLOG_DEBUG("Order {} updated message sent", order->id);
      }
    }
  }

  return 0;
}
