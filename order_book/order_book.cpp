#include "order_book.h"

// Main entry point for matching engine. Consider this the "controller"
std::list<Order *> OrderBook::newOrder(Order * order) {
    totalVolume += order->unfilled_quantity();
    std::list<Order *> updated_orders;

    // When a new buy order comes in that becomes a bid.
    if (order->side == BUY) {
        if (sellBook == nullptr || sellBook->getVolume() == 0) {
            buyBook->addOrder(order);
            // If there are no sell orders & this is a higher bid, move up the bid.
            if (order->limitPrice > bestBid->getPrice()) {
                bestBid = buyBook->get(order->limitPrice);
            }

            return updated_orders;
        }

        // If this buy order is less than the current ask price.
        // (user willing to buy at $5
        // Insert into order book & return.
        if (order->limitPrice < bestAsk->getPrice()) {
            buyBook->addOrder(order);

            return updated_orders;
        }

        // In this exchange implementation, you can only buy at the ask or higher
        // or sell at the bid or lower. There are other ways to do this including
        // Allowing users to trade between the spread but this is configurable.

        // If you are willing to buy at the ask, you will get the trade.
        if (order->limitPrice >= bestAsk->getPrice()) {
            // Iteratively attempt to fill the order until we can't.
            // Then insert the rest of the order into the book.
            while (order->unfilled_quantity() > 0) {
                // @TODO update volume on orderBook.
                updated_orders.merge(this->fillOrder(order));

                // If there are no more orders, return.
                if (sellBook->getVolume() == 0) {
                    // @TODO should probably wrap bestAsk updates in a setter so we can report changes.
                    bestAsk = nullptr;
                    return updated_orders;
                }

                // Because we filled some orders, update the best ask if necessary.
                updateBestAsk();
            }
        }
    } else {

    }
    
    return updated_orders;
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

OrderBook::OrderBook() {
    orderMap = new std::unordered_map<SEQUENCE_ID, Node<Order*> * >();
    buyBook = new Book();
    sellBook = new Book();
    bestBid = nullptr;
    bestAsk = nullptr;
}

// Attempts to fill an order using the buy / sell books.
std::list<Order *> OrderBook::fillOrder(Order* order) {
    if (order->side == BUY) {
        return sellBook->fillOrder(order);
    } else if (order->side == SELL) {
        return buyBook->fillOrder(order);
    }
}

void OrderBook::updateBestAsk() {
    // If we ran out of orders at this price level,
    // Find the next best selling price & make that the ask.
    while (bestAsk->getVolume() == 0) {
        PriceLevel * newBestAsk = sellBook->get(bestAsk->getPrice() + ONE_CENT);
        // sell book get should return null ptr if we retrieve a bad price.
        // or consider adding new prices automagically.
        if (newBestAsk != nullptr && newBestAsk->getVolume() > 0) {
            bestAsk = newBestAsk;
        }
    }
}

int OrderBook::getVolume() {
    return totalVolume;
}

PriceLevel* OrderBook::getBid() {
    return bestBid;
}

PriceLevel* OrderBook::getAsk() {
    return bestAsk;
}
