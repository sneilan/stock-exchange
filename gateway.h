#ifndef _gateway_h
#define _gateway_h

#include "types.h"
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

class Gateway {
  public:
    // Constructor
    Gateway();
    ~Gateway() throw();

    // Public member function
    void put(NewOrderEvent item);
    NewOrderEvent get();

    // a loop that starts a zeromq server and waits for incoming orders
    // Then parsers & puts incoming orders into the ring buffer managed by gateway.
    void run();
  private:
    const char * name = "/gateway_ring_buf";
    int end = 0;       /* write index */
    int start = 0;     /* read index */
    NewOrderEvent* gatewayRingBuf;
    void *socket;
    void *context;
};
#endif