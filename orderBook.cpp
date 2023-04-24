#include "orderBook.h"

Book::Book() {
    limitMap = new std::unordered_map<PRICE, PriceLevel*>();
    int ONE_DOLLAR = 100;
    int ONE_HUNDRED_DOLLARS = ONE_DOLLAR * 100;
    int ONE_CENT = 1;
    for (int price = ONE_DOLLAR; price < ONE_HUNDRED_DOLLARS; price += ONE_CENT) {
        PriceLevel * level = new PriceLevel();
        limitMap->insert(std::make_pair(price, level));
    }
}

PriceLevel* Book::get(PRICE price) {
    return limitMap->at(price);
}

// @TODO concerns for when orders are matched should be handled potentially at this level?
Node<Order *> * Book::insert(Order* order) {
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