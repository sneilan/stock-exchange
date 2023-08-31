#include "book.h"

Book::Book() {
    limitMap = new std::unordered_map<PRICE, PriceLevel*>();
    this->initPriceDataStructures(ONE_DOLLAR, ONE_HUNDRED_DOLLARS);
}

void Book::initPriceDataStructures(int start, int end) {
    for (int price = start; price < end; price += ONE_CENT) {
        PriceLevel * level = new PriceLevel(price);
        limitMap->emplace(price, level);
    }
}

std::list<Order *> Book::fillOrder(Order* order, PriceLevel* level) {
    int initial_quantity = order->unfilled_quantity();

    // here is the bug. we need to fill prices at best bid / ask
    // PriceLevel* level = limitMap->at(order->limitPrice);
    std::list<Order* > updated_orders = level->fillOrder(order);

    totalVolume -= (initial_quantity - order->unfilled_quantity());

    return updated_orders;
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
