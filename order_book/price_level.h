#ifndef price_level_h
#define price_level_h
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cmath>
#include <list>
#include <stdexcept>
#include "../eventstore.h"
#include "../util/types.h"
#include "../util/linked_list.h"

/* Keeps list of orders for each price */
class PriceLevel {
    public:
        Node<Order *> * addOrder(Order* order);
        std::list<Order *> fillOrder(Order* order);
        void cancelOrder(Node<Order*> * node);
        int getVolume();
        int getPrice();

    private:
        // Prices are stored in pennies. $4.56 = 456.
        PRICE limitPrice;
        long totalVolume;
        DoublyLinkedList<Order*> orders;
};

#endif