#ifndef _gateway_h
#define _gateway_h

#include "../util/mmap_wrapper.h"
#include "../util/object_pool.h"
#include "../util/disruptor.h"
#include "../util/types.h"
#include "socket.h"
#include <stdlib.h>
#include <csignal>
#include <cstring>
#include <stdexcept>
#include <fcntl.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <vector>

#define GATEWAY_BUFLEN 100

class Gateway : public SocketServer {
public:
  Gateway(Producer<NewOrderEvent>* incoming_msg_producer,
          Consumer<ORDER_MMAP_OFFSET>* outgoing_message_consumer,
          MMapObjectPool<Order>* order_pool);
  ~Gateway() throw();

  void newClient(int client_id) override;
  void disconnected(int client_id) override;
  void readMessage(int client_id, char *message) override;
  // Whenever a message goes into the outgoing ring buffer
  // this function is called to send a message to the client.
  void handleOutgoingMessage() override;
  void run();
  SSL* getNewSSLObj();

private:
  Producer<NewOrderEvent>* incoming_msg_producer;
  Consumer<ORDER_MMAP_OFFSET> *outgoing_message_consumer;
  MMapObjectPool<Order> *order_pool;
  SSL_CTX *ctx;
  std::vector<SSL*> ssl_pool;
};
#endif
