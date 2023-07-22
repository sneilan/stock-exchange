#ifndef _gateway_h
#define _gateway_h

#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <csignal>
#include <spdlog/spdlog.h>
#include "socket.h"
#include "../util/types.h"
#include "proto/incoming_order.pb.h"
#include "../util/mmap_wrapper.h"

#define GATEWAY_BUFLEN 100

struct NewOrderEvent {
    char side;
    // Stored in pennies
    // $10.05 = 1005
    PRICE limitPrice;
    int quantity;
    char clientId;
    bool stale;
};

class Gateway : public SocketServer {
  public:
    Gateway();
    ~Gateway() throw();

    NewOrderEvent get();

    void newClient(int client_id) override;
    void disconnected(int client_id) override;
    void readMessage(int client_id, char* message) override;
    // void sendMessage(int client_id, char* message);
    // void forceDisconnect(int client_id);

    // a loop that starts a zeromq server and waits for incoming orders
    // Then parsers & puts incoming orders into the ring buffer managed by gateway.
    void run();
  private:
    void put(IncomingOrder item);
    const char * name = "/gateway_ring_buf";
    MMap_Info* mmap_info;
    int end = 0;       /* write index */
    int start = 0;     /* read index */
    NewOrderEvent* gatewayRingBuf;
    int get_mmap_size();
};
#endif
