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

class Gateway : public SocketServer {
public:
  Gateway(Producer<NewOrderEvent>* incoming_msg_producer,
          Consumer<ORDER_MMAP_OFFSET>* outgoing_message_consumer,
          MMapObjectPool<Order>* object_pool);
  ~Gateway() throw();

  void newClient(int client_id) override;
  void disconnected(int client_id) override;
  void readMessage(int client_id, char *message) override;
  void run();

private:
  Producer<NewOrderEvent>* incoming_msg_producer;
};
#endif
