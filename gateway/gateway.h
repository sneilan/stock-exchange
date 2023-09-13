#ifndef _gateway_h
#define _gateway_h

#include "../util/mmap_wrapper.h"
#include "../util/disruptor.h"
#include "../util/types.h"
#include "socket.h"
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define GATEWAY_BUFLEN 100

struct NewOrderEvent {
  char side;
  // Stored in pennies
  // $10.05 = 1005
  PRICE limitPrice;
  int quantity;
  // For now this is the socket id
  // Later on we can create an authentication feature
  // and have actual client Ids.
  int clientId;
  bool stale;
};

class Gateway : public SocketServer {
public:
  Gateway();
  ~Gateway() throw();

  NewOrderEvent* get();

  void newClient(int client_id) override;
  void disconnected(int client_id) override;
  void readMessage(int client_id, char *message) override;
  void run();

private:
  const char *name = "/gateway_ring_buf";
  Producer<NewOrderEvent>* producer;
  Consumer<NewOrderEvent>* consumer;
  // NewOrderEvent *gatewayRingBuf;
  // MMap_Info *mmap_info;
  // int end = 0;   /* write index */
  // int start = 0; /* read index */
  // int get_mmap_size();
};
#endif
