#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <zmq.h>
#include "gateway.h"
#include "eventstore.h"

int main() {
    Gateway * gateway = new Gateway();
    EventStore * eventStore = new EventStore();

    std::cout << "Exchange starting\n";

    pid_t c_pid = fork();
  
    if (c_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (c_pid > 0) {
        // parent
        // Listens to new orders from clients and puts them into the mmap ring buffer maintained by gateway.
        // @TODO should the loop to await data be at a higher level?
        gateway->run();
    }
    else {
        // child
        while (1) {
            // Constantly checking for new orders in the gateway ring buffer.
            // @TODO should I be allocating this?
            NewOrderEvent item = gateway->get();
            if (!item.stale) {
                // Store the event in the event store
                SEQUENCE_ID id = eventStore->newEvent(item.side, item.limitPrice, item.clientId);
                //std::cout << eventStore->get(id).clientId << "\n";
                std::cout << eventStore->size() << "\n";
                //std::cout << "Need to process order from " << item.clientId << " for " << item.limitPrice << " on side " << item.side;
            }
        }
    }

    return 0;
}
