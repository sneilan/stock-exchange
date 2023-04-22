#ifndef _gateway_h
#define _gateway_h
#define GATEWAY_BUFLEN 100

struct NewOrderEvent {
    char side;
    // stored as 55.55
    char limitPrice[5];
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
};
#endif