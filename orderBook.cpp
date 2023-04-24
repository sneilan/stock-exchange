#include "orderBook.h"
#include <iostream>

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