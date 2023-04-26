#include "book.h"

Book::Book() {
    limitMap = new std::unordered_map<PRICE, PriceLevel*>();
    this->allocatePrices(ONE_DOLLAR, ONE_HUNDRED_DOLLARS);
}

void Book::allocatePrices(int start, int end) {
    for (int price = start; price < end; price += ONE_CENT) {
        PriceLevel * level = new PriceLevel();
        limitMap->insert(std::make_pair(price, level));
    }
}

std::list<Order *> Book::fillOrder(Order* order) {
    PriceLevel* level = limitMap->at(order->limitPrice);
    return level->fillOrder(order);
}

Node<Order *> * Book::addOrder(Order* order) {
    totalVolume += order->unfilled_quantity();
    PriceLevel* level = limitMap->at(order->limitPrice);
    return level->addOrder(order);
}

void Book::cancelOrder(Node<Order*> * node) {
    PriceLevel * level = this->get(node->data->limitPrice);

    totalVolume -= node->data->unfilled_quantity();
    level->cancelOrder(node);
}

int Book::getVolume() {
    return totalVolume;
}

PriceLevel* Book::get(PRICE price) {
    return limitMap->at(price);
}
