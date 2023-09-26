#include "eventstore.h"
#include "gateway/gateway.h"
#include "gateway/socket.h"
#include "order_book/order_book.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/version.h>
#include <sys/mman.h>

#include <unistd.h>

// Hacking some stuff out while I nail down sending messages in sockets.
const char *notification = " hello world order notification to the client ";
const char *orderRecievedMsg = "order received";

int main() {
  spdlog::set_level(spdlog::level::debug);
  // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  spdlog::set_pattern("%-5l %E %-16s%-4#%-21! %v");

  Producer<OutgoingMessage> outgoingDisruptor(MAX_OUTGOING_MESSAGES, OUTGOING_MESSAGE_BUFFER);

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
    EventStore *eventStore = new EventStore();
    SPDLOG_INFO("Created EventStore");
    OrderBook *orderBook = new OrderBook();
    SPDLOG_INFO("Created OrderBook");

    OutgoingMessage message;
    message.message = notification;

    OutgoingMessage orderRecieved;
    orderRecieved.message = orderRecievedMsg;

    while (1) {
      // Constantly checking for new orders in the gateway ring buffer.
      NewOrderEvent* item = gateway->get();
      if (item != nullptr) {
        // Store the event in the event store
        // @TODO consider returning an Order* instead of sequence ID.
        SEQUENCE_ID id = eventStore->newEvent(item->side, item->limitPrice,
                                              item->clientId, item->quantity);
        SPDLOG_INFO("Sequence ID is now {} & size is now {}", id,
                    eventStore->size());

        // Get response here & spool information to new ring buffer
        Order *order = eventStore->get(id);
        SPDLOG_INFO("Grabbed order {}", order->id);
        // @TODO This is a call to the matching engine. newOrder name should be
        // more descriptive.
        std::list<Order *> updated_orders = orderBook->newOrder(order);

        orderRecieved.client_id = order->clientId;
        outgoingDisruptor.put(orderRecieved);
          spdlog::debug("Order {} recieved message sent", order->id);

        SPDLOG_INFO("Order book volume is now {}", orderBook->getVolume());
        SPDLOG_INFO("Orders updated are size {}", updated_orders.size());

        for (Order *order : updated_orders) {
          // @TODO Stop using socket ids as client ids. Set up a map
          // between client ids and sockets. Also create a buffer to try
          // to send orders to clients that have disconnected.
          // @TODO send more detailed order information.
          message.client_id = order->clientId;
          outgoingDisruptor.put(message);
          spdlog::debug("Order {} updated message sent", order->id);
        }
      }
    }
  }

  return 0;
}
