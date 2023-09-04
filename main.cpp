#include "eventstore.h"
#include "gateway/gateway.h"
#include "order_book/order_book.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
  Gateway *gateway = new Gateway();

  spdlog::info("Exchange starting");
  spdlog::set_level(spdlog::level::debug);

  pid_t c_pid = fork();

  if (c_pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (c_pid > 0) {
    // Parent
    // Listens to new orders from clients and puts them into the mmap ring
    // buffer maintained by gateway.
    spdlog::info("Gateway starting");
    gateway->run();
  } else {
    // Child
    spdlog::info("Order engine starting");
    EventStore *eventStore = new EventStore();
    spdlog::info("Created EventStore");
    OrderBook *orderBook = new OrderBook();
    spdlog::info("Created OrderBook");

    while (1) {
      // Constantly checking for new orders in the gateway ring buffer.
      NewOrderEvent item = gateway->get();
      if (!item.stale) {
        // Store the event in the event store
        // @TODO consider returning an Order* instead of sequence ID.
        SEQUENCE_ID id = eventStore->newEvent(item.side, item.limitPrice,
                                              item.clientId, item.quantity);
        spdlog::debug("Sequence ID is now {} & size is now {}", id,
                      eventStore->size());

        // Get response here & spool information to new ring buffer
        Order *order = eventStore->get(id);
        spdlog::debug("Grabbed order {}", order->id);
        // @TODO This is a call to the matching engine. newOrder name should be
        // more descriptive.
        std::list<Order *> updated_orders = orderBook->newOrder(order);
        spdlog::debug("Order book volume is now {}", orderBook->getVolume());
        spdlog::debug("Orders updated are size {}", updated_orders.size());

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
