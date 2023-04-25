#include "orderBook.h"

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

void OrderBook::cancelOrder(SEQUENCE_ID id) {
    Node<Order *> * node = orderMap->at(id);
    Order * order = node->data;

    if (order->side == BUY) {
        buyBook->cancelOrder(node);
    } else if (order->side == SELL) {
        sellBook->cancelOrder(node);
    }
}

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

void Book::cancelOrder(Node<Order*> * node) {
    PriceLevel * level = this->get(node->data->limitPrice);
    level->cancelOrder(node);
}

void PriceLevel::cancelOrder(Node<Order*> * node) {
    orders.remove(node);
}

// @TODO concerns for when orders are matched should be handled potentially at this level?
Node<Order *> * PriceLevel::addOrder(Order* order) {
    totalVolume += order->quantity;
    return orders.push_back(order);
}

// Returns a list of filled orders.
std::list<Order *> PriceLevel::fillQuantity(PRICE price, int requested_quantity) {
    // Keep popping orders off at price level until we have either filled quantity
    // or run out of orders.

    std::list<Order *> updated_orders;
    while (orders.get_front() != nullptr && requested_quantity > 0) {
        Node<Order *> * node = orders.get_front();
        int quantity_available = node->data->unfilled_quantity();

        updated_orders.push_back(node->data);

        if (quantity_available > requested_quantity) {
            node->data->filled_quantity += requested_quantity;
            totalVolume -= requested_quantity;
            requested_quantity = 0;
            // If we've filled the order, stop.
            return updated_orders;
        } else if (requested_quantity >= quantity_available) {
            node->data->filled_quantity += quantity_available;
            requested_quantity -= quantity_available;
            totalVolume -= quantity_available;
            orders.remove(node);
        }
    }

    return updated_orders;
}
