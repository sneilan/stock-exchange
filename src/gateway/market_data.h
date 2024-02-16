#ifndef _market_h
#define _market_h

#include "../util/disruptor.h"
#include "../util/mmap_wrapper.h"
#include "../util/types.h"
#include "socket.h"
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define GATEWAY_BUFLEN 100

class MarketData : public SocketServer {
public:
  MarketData(Consumer<L1MarketData> *market_l1_data_consumer);
  ~MarketData() throw();

  void newClient(int client_id) override;
  void disconnected(int client_id) override;
  void readMessage(int client_id, const char *message,
                   int message_size) override;
  void handleOutgoingMessage() override;
  void run();

private:
  Consumer<L1MarketData> *market_l1_data_consumer;
};
#endif
