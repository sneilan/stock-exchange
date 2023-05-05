#ifndef book_h
#define book_h
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cmath>
#include <list>
#include <stdexcept>
#include "../eventstore.h"
#include "../util/types.h"
#include "../util/linked_list.h"
#include "price_level.h"

/* Keeps a list of prices */
class Book {
    public:
        Book();
        PriceLevel* get(PRICE price);
        // Add order to this book.
        Node<Order*> * addOrder(Order* order);
        // Given an order from the other side, attempt to fill it using
        // orders from this book.
        std::list<Order *> fillOrder(Order* order);

        void cancelOrder(Node<Order*> * node);
        int getVolume();
        void initPriceDataStructures(int start, int end);

    private:
        std::unordered_map<PRICE, PriceLevel*> * limitMap;
        int totalVolume = 0;
};

#endif