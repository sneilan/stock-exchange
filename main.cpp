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

int main() {
  spdlog::set_level(spdlog::level::debug);
  // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  spdlog::set_pattern("%-5l %E %-16s%-4#%-21! %v");

  Producer<ORDER_MMAP_OFFSET> outboundMessage(MAX_OUTGOING_MESSAGES,
                                              OUTGOING_MESSAGE_BUFFER);

  SPDLOG_INFO("Allocating EventStore mmap pool..");
  const char *eventstore_buf = "/eventstore_buf";
  MMapObjectPool<Order> *object_pool = new MMapObjectPool<Order>(
      MAX_OPEN_ORDERS, eventstore_buf, IS_CONTROLLER);
  SPDLOG_INFO("Allocated EventStore mmap pool!");

  const char *incoming_msg_buf = "/gateway_ring_buf";
  Producer<NewOrderEvent>* producer = new Producer<NewOrderEvent>(GATEWAY_BUFLEN, incoming_msg_buf);

  Consumer<ORDER_MMAP_OFFSET>* outgoing_message_consumer = new Consumer<ORDER_MMAP_OFFSET>(
      MAX_OUTGOING_MESSAGES, OUTGOING_MESSAGE_BUFFER, OUTGOING_MESSAGE_CONSUMER);

  object_pool = new MMapObjectPool<Order>(MAX_OPEN_ORDERS, eventstore_buf, IS_CLIENT);

  Gateway *gateway = new Gateway(producer,
                                 outgoing_message_consumer,
                                 object_pool);

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

    Consumer<NewOrderEvent>* incoming_order_consumer = new Consumer<NewOrderEvent>(GATEWAY_BUFLEN, incoming_msg_buf, GATEWAY_CONSUMER);
    SPDLOG_INFO("Created consumer for incoming orders.");

    while (1) {
      // Constantly checking for new orders in the gateway ring buffer.
      NewOrderEvent *item = incoming_order_consumer->get();

      if (item == nullptr) {
        continue;
      }

      SPDLOG_DEBUG("Order get for client {} for price {} for "
                   "side {} quantity {}",
                   item->clientId, item->limitPrice, item->side, item->quantity);

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

      outboundMessage.put(offset);
      // State of order is based on how many fills.
      SPDLOG_DEBUG("Order {} recieved message sent", order->id);

      SPDLOG_INFO("Order book volume is now {}", orderBook->getVolume());
      SPDLOG_INFO("Orders updated are size {}", updated_orders.size());

      for (Order *order : updated_orders) {
        // @TODO Stop using socket ids as client ids. Set up a map
        // between client ids and sockets. Also create a buffer to try
        // to send orders to clients that have disconnected.
        outboundMessage.put(object_pool->pointer_to_offset(order));
        SPDLOG_DEBUG("Order {} updated message sent", order->id);
      }
    }
  }

  // @TODO create signal handler to clean up

  return 0;
}
