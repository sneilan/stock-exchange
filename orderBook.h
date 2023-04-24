#ifndef order_book_h_
#define order_book_h_
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <list>
#include "eventstore.h"
#include "types.h"

struct Order {
    // Define the structure of an Order
    // ...
    SEQUENCE_ID id;
    int quantity;
};

class PriceLevel {
    private:
    // Prices are stored in pennies. $4.56 = 456.
    PRICE limitPrice;
    long totalVolume;
    // @TODO use custom allocator to manage deletion / creation of orders.
    std::list<Order*> orders;
};

class Book {
    private:
    std::unordered_map<PRICE, PriceLevel*>* limitMap;
    public:
    Book() {
        limitMap = new std::unordered_map<PRICE, PriceLevel*>();
        
    }
};

class OrderBook {
    private:
    Book* buyBook;
    Book* sellBook;
    PriceLevel* bestBid;
    PriceLevel* bestOffer;
    std::unordered_map<SEQUENCE_ID, Order*>* orderMap;

    public:
    OrderBook() {
        orderMap = new std::unordered_map<SEQUENCE_ID, Order*>();
    }
};
#endif