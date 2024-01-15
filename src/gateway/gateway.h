#ifndef _gateway_h
#define _gateway_h

#include "../util/disruptor.h"
#include "../util/mmap_wrapper.h"
#include "../util/object_pool.h"
#include "../util/types.h"
#include "socket.h"
#include "util/websocket.h"
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#define GATEWAY_BUFLEN 100

using namespace std;

class Gateway : public SocketServer {
public:
  Gateway(Producer<NewOrderEvent> *incoming_msg_producer,
          Consumer<ORDER_MMAP_OFFSET> *outgoing_message_consumer,
          MMapObjectPool<Order> *order_pool);

  // Used for testing.
  Gateway();
  ~Gateway() throw();

  void newClient(int client_id) override;
  void disconnected(int client_id) override;
  void readMessage(int client_id, const char *message) override;
  // Whenever a message goes into the outgoing ring buffer
  // this function is called to send a message to the client.
  void handleOutgoingMessage() override;
  void run();

private:
  Producer<NewOrderEvent> *incoming_msg_producer;
  Consumer<ORDER_MMAP_OFFSET> *outgoing_message_consumer;
  MMapObjectPool<Order> *order_pool;
  vector<bool> connected_webclients;
};
#endif
