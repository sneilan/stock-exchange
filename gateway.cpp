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
