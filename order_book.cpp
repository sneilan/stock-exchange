#include "order_book.h"

OrderBook::OrderBook() {
    orderMap = new std::unordered_map<SEQUENCE_ID, Node<Order*> * >();
    buyBook = new Book();
    sellBook = new Book();
    bestBid = nullptr;
    bestOffer = nullptr;
}

std::list<Order *> OrderBook::newOrder(Order * order) {
    totalVolume += order->unfilled_quantity();
    std::list<Order *> updated_orders;

    if (order->side == BUY) {
        std::cout << "Inserting new buy order for price " << order->limitPrice << "\n";
        auto *ptr = buyBook->insert(order);
        orderMap->insert(std::make_pair(order->id, ptr));

        // If we run out of bids or whatever at this level, move best bid around.
        // if best bid becomes null b/c no orders then it's just whatever it is.

        // if buying, try to fill in orders
        // move the bids / asks around.
        // If this buy price is lo
        //if (order->limitPrice < bestBid->getPrice()) {
          //  bestBid = buyBook->get(order->limitPrice);
        //}
    } else {
        std::cout << "Inserting new sell order for price " << order->limitPrice << "\n";
        auto *ptr = sellBook->insert(order);
        orderMap->insert(std::make_pair(order->id, ptr));

        // if (order->limitPrice > bestOffer->getPrice()) {
           // bestOffer = sellBook->get(order->limitPrice);
        // }
    }
    
    return updated_orders;
}

PriceLevel* OrderBook::getBid() {
    return bestBid;
}

PriceLevel* OrderBook::getAsk() {
    return bestOffer;
}

void OrderBook::cancelOrder(SEQUENCE_ID id) {
    Node<Order *> * node = orderMap->at(id);
    totalVolume -= node->data->unfilled_quantity();

    Order * order = node->data;

    if (order->side == BUY) {
        buyBook->cancelOrder(node);
    } else if (order->side == SELL) {
        sellBook->cancelOrder(node);
    }
}

int OrderBook::getVolume() {
    return totalVolume;
}

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

PriceLevel* Book::get(PRICE price) {
    return limitMap->at(price);
}

Node<Order *> * Book::insert(Order* order) {
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

void PriceLevel::cancelOrder(Node<Order*> * node) {
    totalVolume -= node->data->unfilled_quantity();
    orders.remove(node);
}

int PriceLevel::getPrice() {
    return limitPrice;
}

Node<Order *> * PriceLevel::addOrder(Order* order) {
    totalVolume += order->quantity;
    return orders.push_back(order);
}

// Returns a list of filled orders.
std::list<Order *> PriceLevel::fillQuantity(int requested_quantity) {
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

int PriceLevel::getVolume() {
    return totalVolume;
}