#include "orderBook.h"
#include <iostream>
#include <algorithm>

Book::Book() {
    limitMap = new std::unordered_map<PRICE, PriceLevel*>();
    int ONE_DOLLAR = 100;
    int ONE_HUNDRED_DOLLARS = ONE_DOLLAR * 100;
    int ONE_CENT = 1;
    for (int price = ONE_DOLLAR; price < ONE_HUNDRED_DOLLARS; price += ONE_CENT) {
        PriceLevel * level = new PriceLevel();
        limitMap->insert(std::make_pair(price, level));
    }
    std::cout << "There are " << limitMap->size() << " prices in book.\n";
}

PriceLevel* Book::get(PRICE price) {
    std::cout << "Retrieving price from book via get " << price << "\n";
    return limitMap->at(price);
}


OrderBook::OrderBook() {
    orderMap = new std::unordered_map<SEQUENCE_ID, Node<Order*> * >();
    buyBook = new Book();
    sellBook = new Book();
    // Default to best bid / ask at 50.01 & 50.00
    bestBid = buyBook->get(5001);
    bestOffer = sellBook->get(5000);
}

void OrderBook::newOrder(Order * order) {
    if (order->side == BUY) {
        std::cout << "Inserting new buy order for price " << order->limitPrice << "\n";
        auto *ptr = buyBook->insert(order);
        orderMap->insert(std::make_pair(order->id, ptr));
    } else {
        std::cout << "Inserting new sell order for price " << order->limitPrice << "\n";
        auto *ptr = sellBook->insert(order);
        orderMap->insert(std::make_pair(order->id, ptr));
    }
}

// @TODO concerns for when orders are matched should be handled potentially at this level?
Node<Order *> * Book::insert(Order* order) {
    std::cout << "Retrieving price from Book via insert" << order->limitPrice << "\n";
    PriceLevel* level = limitMap->at(order->limitPrice);
    return level->addOrder(order);
}

// @TODO concerns for when orders are matched should be handled potentially at this level?
Node<Order *> * PriceLevel::addOrder(Order* order) {
    totalVolume += order->quantity;
    return orders._push_back(order);
}

int PriceLevel::fillQuantity(PRICE price, int requested_quantity) {
    // Keep popping orders off at price level until we have either filled quantity
    // or run out of orders.

    int filled_quantity = 0;
    while not orders_list.is_empty() and filled_quantity < requested_quantity:
        current_quantity = orders_list.head.quantity
        
        if filled_quantity + current_quantity > requested_quantity:
            current_quantity = requested_quantity - filled_quantity
        
        filled_quantity += current_quantity
        num_orders_filled += 1
        orders_list.pop_front()


    while (orders.get_front() != nullptr && filled_quantity < requested_quantity) {
        Node<Order *> * node = orders.get_front();
        if (node->data->quantity <= requested_quantity) {
            requested_quantity -= node->data->quantity;
            node->data->filled_quantity = node->data->quantity;
            totalVolume -= node->data->quantity;
            orders.pop_front(); // @TODO notify user their order has been filled
        } else if (node->data->quantity > requested_quantity) {
            node->data->filled_quantity += requested_quantity;
            totalVolume -= requested_quantity;
            requested_quantity = 0;
        }
    }

    // If I have 10 shares I'd like to fill and a node has 20 shares available
    // quantityToBeFilled should be 0, filled_quantity will be 10
    quantityToBeFilled -= std::min(node->data->filled_quantity, quantityToBeFilled);

    totalVolume += order->quantity;
    return orders._push_back(order);
}

//void OrderBook::cancelOrder(SEQUENCE_ID id) {
//    orderMap->at(id);
//}