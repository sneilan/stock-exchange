#ifndef order_book_h_
#define order_book_h_
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cmath>
#include <list>
#include <stdexcept>
#include "eventstore.h"
#include "types.h"
#include "util/linked_list.h"

class PriceLevel {
    private:
        // Prices are stored in pennies. $4.56 = 456.
        PRICE limitPrice;
        long totalVolume;
        DoublyLinkedList<Order*> orders;
    public:
        Node<Order *> * addOrder(Order* order);
        std::list<Order *> fillQuantity(PRICE price, int quantity);
};

class Book {
    public:
        std::unordered_map<PRICE, PriceLevel*> * limitMap;
        Book();
        PriceLevel* get(PRICE price);
        Node<Order*> * insert(Order* order);
        void cancelOrder(Node<Order*> * node);
};

class OrderBook {
    private:
        Book* buyBook;
        Book* sellBook;
        PriceLevel* bestBid;
        PriceLevel* bestOffer;
        // Create an unordered map of sequence ids to iterators
        // Then we can later implement a custom allocator to manage these iterators
        // so that they don't blow up the heap.
        // In a v2 consider implementing my own linked list so we don't need pointers to iterators.
        std::unordered_map<SEQUENCE_ID, Node<Order*> * >* orderMap;

    public:
        OrderBook();

        void newOrder(Order * order); // give a list of orders matched or none at all.
        void cancelOrder(SEQUENCE_ID id);
};

#endif