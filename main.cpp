#include "eventstore.h"
#include "gateway/gateway.h"
#include "order_book/order_book.h"
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
  // spdlog::set_pattern("[%l] %E %-16s%-4#%-21! %v");
  spdlog::set_pattern("[%l] %E %-16s%-4#%-21! %v");

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

    while (1) {
      // Constantly checking for new orders in the gateway ring buffer.
      NewOrderEvent item = gateway->get();
      if (!item.stale) {
        // Store the event in the event store
        // @TODO consider returning an Order* instead of sequence ID.
        SEQUENCE_ID id = eventStore->newEvent(item.side, item.limitPrice,
                                              item.clientId, item.quantity);
        SPDLOG_INFO("Sequence ID is now {} & size is now {}", id,
                      eventStore->size());

        // Get response here & spool information to new ring buffer
        Order *order = eventStore->get(id);
        SPDLOG_INFO("Grabbed order {}", order->id);
        // @TODO This is a call to the matching engine. newOrder name should be
        // more descriptive.
        std::list<Order *> updated_orders = orderBook->newOrder(order);
        SPDLOG_INFO("Order book volume is now {}", orderBook->getVolume());
        SPDLOG_INFO("Orders updated are size {}", updated_orders.size());

        // @TODO issue with sending orders.. :(
        // I cannot send orders from the child process
        // because new sockets are being opened on the parent process gateway.
        // The child process task_struct in the kernel does not know
        // what was opened in the parent process task_struct after the call
        // to fork()
        // for (Order *order : updated_orders) {
        //   const char *message = "order updated";
        //   // @TODO send updated order information to the clients via another
        //   // ring buffer. Another process will read from this ring buffer and
        //   // send data to the client.
        //   // @TODO Stop using socket ids as client ids. Set up a map
        //   // between client ids and sockets. Also create a buffer to try
        //   // to send orders to clients that have disconnected.
        //   gateway->sendMessage(order->clientId, message);
        //   spdlog::debug("Orders updated message sent");
        // }
      }
    }
  }

  return 0;
}
