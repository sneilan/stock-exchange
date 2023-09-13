#include "gateway.h"

Gateway::Gateway() {
  mmap_info = init_mmap(name, get_mmap_size());
  gatewayRingBuf = (NewOrderEvent *)mmap_info->location;

  // Initialize all orders to stale
  for (int i = 0; i < GATEWAY_BUFLEN; i++) {
    gatewayRingBuf[i].stale = true;
  }
}

Gateway::~Gateway() throw() { delete_mmap(mmap_info); }

NewOrderEvent Gateway::get() {
  // Copy what is in the ring buffer into a new structure.
  // Eventually we can stream orders directly from the network card
  // to the mmap buffer for increased performance.
  NewOrderEvent item = gatewayRingBuf[start];

  if (!item.stale) {
    SPDLOG_DEBUG("Ring buffer Order retrieved for client {} for price {} for "
                 "side {} quantity {}",
                 item.clientId, item.limitPrice, item.side, item.quantity);
  }
  // Mark the old copy of new order event in ring buffer as stale.
  gatewayRingBuf[start].stale = true;

  // @TODO proper lmax algo does not increment start until another process tells
  // it to. See https://martinfowler.com/articles/lmax.html
  start++;
  start %= GATEWAY_BUFLEN;
  return item;
}

void Gateway::readMessage(int client_id, char *message) {
  SPDLOG_INFO("Read message from {}", client_id);

  // Not building an authentication system yet
  // so just sending trades back to clients by socket id.
  gatewayRingBuf[end].clientId = client_id;
  gatewayRingBuf[end].limitPrice = ((NewOrderEvent *)message)->limitPrice;
  gatewayRingBuf[end].side = ((NewOrderEvent *)message)->side;
  gatewayRingBuf[end].quantity = ((NewOrderEvent *)message)->quantity;
  gatewayRingBuf[end].stale = false;

  SPDLOG_INFO("Ring buffer Order recieved from client {} for price {} for "
              "side {} quantity {}",
              gatewayRingBuf[end].clientId, gatewayRingBuf[end].limitPrice,
              gatewayRingBuf[end].side, gatewayRingBuf[end].quantity);

  end++;

  end %= GATEWAY_BUFLEN;

  const char *msg = "order received";
  sendMessage(client_id, msg);
}

void Gateway::newClient(int client_id) {
  SPDLOG_INFO("New client {}", client_id);
  const char *msg = "Welcome new client";
  if (!sendMessage(client_id, msg)) {
    // @TODO perhaps sendMessage can handle what happens if a client disconnects
    // Then call our disconnected handler and let us know so we don't have to do
    // an error handling pattern everywhere.
    forceDisconnect(client_id);
  }
}

void Gateway::disconnected(int client_id) {
  SPDLOG_INFO("Client disconnected {}", client_id);
}

int Gateway::get_mmap_size() {
  return sizeof(NewOrderEvent) * (GATEWAY_BUFLEN);
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
