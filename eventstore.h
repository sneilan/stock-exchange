#ifndef _eventstore_h
#define _eventstore_h

#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <unordered_map>
#include <stdexcept>


#define EVENTSTORE_BUFLEN 10000
#define SEQUENCE_ID unsigned long long

struct Event {
    SEQUENCE_ID sequence;
    char side;
    // stored as 55.55
    char limitPrice[5];
    char clientId;
    // Something else related to the event like if it's cancelled, new order, order modification.
    // for now lets focus on sequences.
    // And status of the order like accepted, matched, cancelled
    // maybe even last modified date?
    // lets also skip order modification as a feature for
};

class EventStore {
    public:
    EventStore();
    ~EventStore();
    void newEvent(char side, char limitPrice[5], char clientId);

    private:
    size_t size;
    void* ptr;
    std::unordered_map<SEQUENCE_ID, Event> *  eventStoreBuf;
    int fd;
};



#endif