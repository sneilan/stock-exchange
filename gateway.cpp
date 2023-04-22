#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <zmq.h>

#define BUFLEN 100

struct NewOrderEvent {
    int side;
    int limitPrice;
    int clientId;
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

    void put(NewOrderEvent item) {
        gatewayRingBuf[end].clientId = item.clientId;
        gatewayRingBuf[end].limitPrice = item.limitPrice;
        gatewayRingBuf[end].side = item.side;
        gatewayRingBuf[end].stale = false;
        end++;

        std::cout << end << "\n";
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
    item.clientId = 0;
    item.limitPrice = 0;
    item.side = 0;

    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    while (1) {
        char buffer [3];
        zmq_recv (responder, buffer, 10, 0);
        item.clientId = (int)buffer[0];
        item.limitPrice = (int)buffer[1];
        item.side = (int)buffer[2];
        gateway->put(item);
        std::cout << "Order recieved from client " << item.clientId << " for price " << item.limitPrice << " for side " << item.side;
        zmq_send (responder, "ack", 5, 0);
    }

    return 0;
}