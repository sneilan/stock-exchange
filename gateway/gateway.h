#ifndef _gateway_h
#define _gateway_h

#include "../util/types.h"
#include "proto/incoming_order.pb.h"
#include "socket.h"
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
    int end = 0;       /* write index */
    int start = 0;     /* read index */
    NewOrderEvent* gatewayRingBuf;
    void *socket;
    void *context;
};
#endif
