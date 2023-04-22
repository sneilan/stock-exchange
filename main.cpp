#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <zmq.h>
#include "gateway.h";

void runGateway(Gateway* gateway) {
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    NewOrderEvent item;
    char buffer[7];
    while (1) {
        // clientId: 0-9, limitPrice: xx.xx, side: b or s
        // 0xx.xxb
        std::string str = std::string(buffer);
        zmq_recv (responder, buffer, 7, 0);
        item.clientId = buffer[0];
        std::memcpy(item.limitPrice, &buffer[1], 5);
        item.side = buffer[6];
        gateway->put(item);
        std::cout << "Order recieved from client " << item.clientId << " for price " << item.limitPrice << " for side " << item.side << "\n";
        zmq_send (responder, "ack", 3, 0);
    }
}

int main() {
    Gateway * gateway = new Gateway();

    pid_t c_pid = fork();
  
    if (c_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (c_pid > 0) {
        // parent
        // Listens to new orders from clients and puts them into the mmap ring buffer maintained by gateway.
        runGateway(gateway);
    }
    else {
        // child
        while (1) {
            // Constantly checking for new orders in the gateway ring buffer.
            // @TODO should I be allocating this?
            NewOrderEvent item = gateway->get();
            if (!item.stale) {
                std::cout << "Need to process order from " << item.clientId << " for " << item.limitPrice << " on side " << item.side;
            }
        }
    }

    return 0;
}
