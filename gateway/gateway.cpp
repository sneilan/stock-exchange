#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <csignal>
#include "gateway.h"
#include "../util/spdlog/spdlog.h"
#include "socket.h"

Gateway::Gateway() {
  int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
  ftruncate(fd, sizeof(NewOrderEvent) * (GATEWAY_BUFLEN));
  gatewayRingBuf = (NewOrderEvent*)mmap( NULL, sizeof(NewOrderEvent) * (GATEWAY_BUFLEN), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  memset(gatewayRingBuf, 0, sizeof(NewOrderEvent) * GATEWAY_BUFLEN); // clear shared memory block

  // Initialize all orders to stale
  for (int i = 0; i < GATEWAY_BUFLEN; i++) {
    gatewayRingBuf[i].stale = true;
  }
}

Gateway::~Gateway() throw() {
    munmap(gatewayRingBuf, sizeof(NewOrderEvent) * (GATEWAY_BUFLEN));
    shm_unlink(name);
}

NewOrderEvent Gateway::get() {
    // Copy what is in the ring buffer into a new structure.
    NewOrderEvent item = gatewayRingBuf[start];

    spdlog::debug("Ring buffer Order retrieved for client {} for price {} for side {}", item.clientId, item.limitPrice, item.side);
    // Mark the old copy of new order event in ring buffer as stale.
    gatewayRingBuf[start].stale = true;
    start++;
    start %= GATEWAY_BUFLEN;
    return item;
}

void Gateway::newClient(int client_id) {

}

void Gateway::disconnected(int client_id) {

}

// @TODO instead of recreating item each time, pass in values perhaps?
// one less copy per call.
void Gateway::put(IncomingOrder item) {
    gatewayRingBuf[end].clientId = item.clientid();
    gatewayRingBuf[end].limitPrice = item.limitprice();
    gatewayRingBuf[end].side = item.side()[0];
    gatewayRingBuf[end].stale = false;

    end++;

    spdlog::debug("Ring buffer Order recieved from client {} for price {} for side {}", item.clientid(), item.limitprice(), item.side()[0]);

    end %= GATEWAY_BUFLEN;
}

void Gateway::readMessage(int socket_client_id, char* message) {
  IncomingOrder incomingOrder;

  if (!incomingOrder.ParseFromString(message)) {
    std::cerr << "Failed to parse incoming order." << std::endl;
    return;
  }

  int limitPrice = incomingOrder.limitprice();
  std::string side = incomingOrder.side();
  int clientId = incomingOrder.clientid();
  int quantity = incomingOrder.quantity();

}

void Gateway::sendMessage(int client_id, char* message) {

}

void Gateway::forceDisconnect(int client_id) {

}

void Gateway::run() {
    /* 
    // @TODO
    Socket server should have the following events
     1) New connection
     2) Disconnect
     3) Reading data from client
    Expose the following methods
     1) Sending data to a client
     2) Getting a list of open clients.
     3) Force closing a client connection
    Something else can handle parsing data from clients.
    */

    bindSocket(8888);
    listenToSocket();
}
