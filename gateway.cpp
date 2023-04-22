#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>

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
    item.clientId = 5;
    item.limitPrice = 6;
    item.side = 0;

    for (int i = 0; i < 1000; i += 1) {
        item.clientId = i;
        gateway->put(item);
        // std::cout << gateway->get().clientId << "\n";
    }

    return 0;
}