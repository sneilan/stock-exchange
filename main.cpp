#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <zmq.h>
#include "gateway.h"
#include "eventstore.h"
#include "order_book/order_book.h"
#include "util/spdlog/spdlog.h"

int main() {
    Gateway * gateway = new Gateway();
    EventStore * eventStore = new EventStore();

    spdlog::info("Exchange starting");

    pid_t c_pid = fork();
  
    if (c_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (c_pid > 0) {
        // Parent
        // Listens to new orders from clients and puts them into the mmap ring buffer maintained by gateway.
        // @TODO Remove ZeroMQ and replace with raw sockets for a proper client-server comms
        // Message brokers are the wrong solution but they get the job done for testing.
        gateway->run();
    } else {
        // Cshild
        OrderBook* orderBook = new OrderBook();

        while (1) {
            // Constantly checking for new orders in the gateway ring buffer.
            NewOrderEvent item = gateway->get();
            if (!item.stale) {
                // Store the event in the event store
                SEQUENCE_ID id = eventStore->newEvent(item.side, item.limitPrice, item.clientId, item.quantity);
                spdlog::debug("Sequence ID is now", id);
                spdlog::debug("Size is now", eventStore->size());

                Order * order = eventStore->get(id);

                spdlog::debug("Price of order is {}", item.limitPrice);

                // Get response here & spool information to new ring buffer
                std::list<Order *> updated_orders = orderBook->newOrder(order);

                // @TODO send updated order information to the clients via another ring buffer.
                // Another process will read from this ring buffer and send data to the client.
            }
        }
    }

    return 0;
}
