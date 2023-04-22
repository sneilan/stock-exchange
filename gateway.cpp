#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <zmq.h>
#include <csignal>
#include "gateway.h"

Gateway::Gateway() {
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(NewOrderEvent) * BUFLEN);
    gatewayRingBuf = (NewOrderEvent*)mmap( NULL, sizeof(NewOrderEvent) * BUFLEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // Initialize all orders to stale
    for (int i = 0; i < BUFLEN; i++) {
        gatewayRingBuf[i].stale = true;
    }
}

Gateway::~Gateway() throw() {
    munmap(gatewayRingBuf, sizeof(NewOrderEvent) * BUFLEN);
    shm_unlink(name);
    std::cout << "Destructor called";
}

// @TODO instead of recreating item each time, pass in values perhaps?
// one less copy per call.
void Gateway::put(NewOrderEvent item) {
    gatewayRingBuf[end].clientId = item.clientId;
    std::memcpy(gatewayRingBuf[end].limitPrice, item.limitPrice, 5);
    gatewayRingBuf[end].side = item.side;
    gatewayRingBuf[end].stale = false;
    std::cout << gatewayRingBuf[end].limitPrice << "\n";

    end++;

    end %= BUFLEN;
}

NewOrderEvent Gateway::get() {
    NewOrderEvent item = gatewayRingBuf[start];
    gatewayRingBuf[start].stale = true;
    start++;
    start %= BUFLEN;
    return item;
}

void Gateway::run() {
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
        this->put(item);
        std::cout << "Order recieved from client " << item.clientId << " for price " << item.limitPrice << " for side " << item.side << "\n";
        zmq_send (responder, "ack", 3, 0);
    }
}
