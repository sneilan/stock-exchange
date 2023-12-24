#include "market_data.h"

MarketData::MarketData(Consumer<L1MarketData>* market_l1_data_consumer) {
  this->market_l1_data_consumer = market_l1_data_consumer;
}

MarketData::~MarketData() throw() {}

// I don't care about these for now because I am implementing authentication later.
// For now anyone can get market data if they connect.

void MarketData::newClient(int client_id) {
  const char *msg = "Welcome new market data consumer";
  if (!sendMessage(client_id, const_cast<char *>(msg), strlen(msg))) {
    forceDisconnect(client_id);
  }
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
  SPDLOG_DEBUG("Sent Mkt {} Value {} ", market_data->type, market_data->val);
}

void MarketData::run() {
  char * port = getenv("MARKET_PORT");
  bindSocket(atoi(port));
  listenToSocket();
}
