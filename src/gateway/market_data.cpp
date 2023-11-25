#include "market_data.h"

MarketData::MarketData(Consumer<L1MarketData>* market_l1_data_consumer) {
  this->market_l1_data_consumer = market_l1_data_consumer;
}

MarketData::~MarketData() throw() {}

// I don't care about these for now because I am implementing authentication later.
// For now anyone can get market data if they connect.
void MarketData::newClient(int client_id) {
  // @TODO send nice message to client.
  // sendMessage(int client_id, char *message, int message_size)
}
void MarketData::disconnected(int client_id) {};
void MarketData::readMessage(int client_id, char *message) {};

void MarketData::handleOutgoingMessage() {
  L1MarketData *market_data = market_l1_data_consumer->get();
  if (market_data == nullptr) {
    return;
  }

  // For every client, send market data.
  sendMessageToAllClients((char*)market_data, sizeof(L1MarketData));
}

void MarketData::run() {
  bindSocket(8889);
  listenToSocket();
}
