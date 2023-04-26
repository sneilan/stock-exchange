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
        // parent
        // Listens to new orders from clients and puts them into the mmap ring buffer maintained by gateway.
        // @TODO should the loop to await data be at a higher level?
        gateway->run();
    } else {
        // child
        OrderBook* orderBook = new OrderBook();

        while (1) {
            // Constantly checking for new orders in the gateway ring buffer.
            NewOrderEvent item = gateway->get();
            if (!item.stale) {
                // Store the event in the event store
                SEQUENCE_ID id = eventStore->newEvent(item.side, item.limitPrice, item.clientId, item.quantity);
                // std::cout << "Sequence ID is now " << id << "\n";
                // std::cout << "size is now " << eventStore->size() << "\n";
                Order * order = eventStore->get(id);

                spdlog::debug("Price of order is {}", item.limitPrice);

                // get response here & spool information to new ring buffer
                // create new process that reads from new ring buffer & sends to clients.
                orderBook->newOrder(order);

                //std::cout << eventStore->get(id)->clientId << "\n";
            }
        }
    }

    return 0;
}
