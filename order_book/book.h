#ifndef book_h
#define book_h
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cmath>
#include <list>
#include <stdexcept>
#include "../eventstore.h"
#include "../types.h"
#include "../util/linked_list.h"
#include "price_level.h"

/* Keeps a list of prices */
class Book {
    public:
        Book();
        PriceLevel* get(PRICE price);
        Node<Order*> * insert(Order* order);
        void cancelOrder(Node<Order*> * node);
        int getVolume();
        void allocatePrices(int start, int end);

    private:
        std::unordered_map<PRICE, PriceLevel*> * limitMap;
        int totalVolume = 0;
};

#endif