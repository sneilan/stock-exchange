#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include "gateway/gateway.h"
#include "eventstore.h"
#include "order_book/order_book.h"
#include <spdlog/spdlog.h>

int main() {
    Gateway * gateway = new Gateway();
    EventStore * eventStore = new EventStore();

    spdlog::info("Exchange starting");
    spdlog::set_level(spdlog::level::debug);

    pid_t c_pid = fork();
  
    if (c_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (c_pid > 0) {
        // Parent
        // Listens to new orders from clients and puts them into the mmap ring buffer maintained by gateway.
        gateway->run();
    } else {
        // Cshild
        OrderBook* orderBook = new OrderBook();

        while (1) {
            // Constantly checking for new orders in the gateway ring buffer.
            NewOrderEvent item = gateway->get();
            if (!item.stale) {
                // Store the event in the event store
                // @TODO consider returning an Order* instead of sequence ID.
                SEQUENCE_ID id = eventStore->newEvent(item.side, item.limitPrice, item.clientId, item.quantity);
                spdlog::debug("Sequence ID is now {} & size is now {}", id, eventStore->size());

                // Get response here & spool information to new ring buffer
                Order * order = eventStore->get(id);
                // @TODO This is a call to the matching engine. newOrder name should be more descriptive.
                std::list<Order *> updated_orders = orderBook->newOrder(order);
                spdlog::debug("Order book volume is now {}", orderBook->getVolume());

                // @TODO send updated order information to the clients via another ring buffer.
                // Another process will read from this ring buffer and send data to the client.
            }
        }
    }

    return 0;
}
