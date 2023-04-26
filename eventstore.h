#ifndef _eventstore_h
#define _eventstore_h

#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <unordered_map>
#include <stdexcept>
#include "./util/types.h"

#define EVENTSTORE_BUFLEN 10000


class EventStore {
    public:
        EventStore();
        ~EventStore();
        SEQUENCE_ID newEvent(SIDE side, PRICE limitPrice, char clientId, int quantity);
        Order* get(SEQUENCE_ID id);
        size_t size();

    private:
        size_t shared_mem_size;
        void* ptr;
        int fd;

        SEQUENCE_ID sequence;
        std::unordered_map<SEQUENCE_ID, Order> *  eventStoreBuf;
};

#endif