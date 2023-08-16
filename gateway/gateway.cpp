#include "gateway.h"

Gateway::Gateway() {
  mmap_info = init_mmap(name, get_mmap_size());
  gatewayRingBuf = (NewOrderEvent*)mmap_info->location;

  // Initialize all orders to stale
  for (int i = 0; i < GATEWAY_BUFLEN; i++) {
    gatewayRingBuf[i].stale = true;
  }
}

Gateway::~Gateway() throw() {
  delete_mmap(mmap_info);
}

NewOrderEvent Gateway::get() {
    // Copy what is in the ring buffer into a new structure.
    NewOrderEvent item = gatewayRingBuf[start];

    if (!item.stale) {
      spdlog::debug("Ring buffer Order retrieved for client {} for price {} for side {}", item.clientId, item.limitPrice, item.side);
    }
    // Mark the old copy of new order event in ring buffer as stale.
    gatewayRingBuf[start].stale = true;

    // @TODO proper lmax algo does not increment start until another process tells it to.
    // See https://martinfowler.com/articles/lmax.html
    start++;
    start %= GATEWAY_BUFLEN;
    return item;
}

void Gateway::newClient(int client_id) {
  spdlog::info("New client {}", client_id);
  const char * msg = "Welcome new client";
  if (!sendMessage(client_id, msg)) {
    // @TODO perhaps sendMessage can handle what happens if a client disconnects
    // Then call our disconnected handler and let us know so we don't have to do
    // an error handling pattern everywhere.
    forceDisconnect(client_id);
  }
}

void Gateway::disconnected(int client_id) {
  spdlog::info("Client disconnected {}", client_id);
}

int Gateway::get_mmap_size() {
  return sizeof(NewOrderEvent) * (GATEWAY_BUFLEN);
}

// @TODO instead of recreating item each time, pass in values perhaps?
// one less copy per call.
void Gateway::put(char* item) {
    // gatewayRingBuf[end].clientId = item.clientid();
    // gatewayRingBuf[end].limitPrice = item.limitprice();
    // gatewayRingBuf[end].side = item.side()[0];
    // gatewayRingBuf[end].quantity = item.quantity();
    // gatewayRingBuf[end].stale = false;

    end++;

    // spdlog::info("Ring buffer Order recieved from client {} for price {} for side {}", item.clientid(), item.limitprice(), item.side()[0]);

    end %= GATEWAY_BUFLEN;
}

void Gateway::readMessage(int client_id, char* message) {
  // IncomingOrder incomingOrder;

  spdlog::info("Read message from {}", client_id);

  // if (!incomingOrder.ParseFromString(message)) {
    spdlog::info("Failed to parse incoming order.");
    return;
  // }

  // put(incomingOrder);

  const char * msg = "order received";
  sendMessage(client_id, msg);
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
