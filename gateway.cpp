#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <zmq.h>
#include <csignal>

#define BUFLEN 100

struct NewOrderEvent {
    char side;
    // stored as 55.55
    char limitPrice[5];
    char clientId;
    bool stale;
};

class Gateway {
    private:
    const char* name = "/gateway_ring_buf";
    int end = 0;       /* write index */
    int start = 0;     /* read index */
    NewOrderEvent* gatewayRingBuf;

    public:
    Gateway() {
        int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
        ftruncate(fd, sizeof(NewOrderEvent) * BUFLEN);
        gatewayRingBuf = (NewOrderEvent*)mmap( NULL, sizeof(NewOrderEvent) * BUFLEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    }

    ~Gateway() {
        munmap(gatewayRingBuf, sizeof(NewOrderEvent) * BUFLEN);
        shm_unlink(name);
    }

    // @TODO instead of recreating item each time, pass in values perhaps?
    // one less copy per call.
    void put(NewOrderEvent item) {
        gatewayRingBuf[end].clientId = item.clientId;
        std::memcpy(gatewayRingBuf[end].limitPrice, item.limitPrice, 5);
        gatewayRingBuf[end].side = item.side;
        gatewayRingBuf[end].stale = false;
        std::cout << gatewayRingBuf[end].limitPrice << "\n";

        end++;

        end %= BUFLEN;
    }

    NewOrderEvent get(){
        NewOrderEvent item = gatewayRingBuf[start++];
        start %= BUFLEN;
        return item;
    }
};

int main () {
    Gateway * gateway = new Gateway();
    NewOrderEvent item;

    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    char buffer[7];
    while (1) {
        // clientId: 0-9, limitPrice: xx.xx, side: b or s
        // 0xx.xxb    
        std::string str = std::string(buffer);
        zmq_recv (responder, buffer, 7, 0);
        item.clientId = buffer[0];
        std::memcpy(item.limitPrice, &buffer[1], 5);
        item.side = buffer[6];
        gateway->put(item);
        std::cout << "Order recieved from client " << item.clientId << " for price " << item.limitPrice << " for side " << item.side;
        zmq_send (responder, "ack", 5, 0);
    }

    return 0;
}