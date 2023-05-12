#include "order_book.h"
#include "../util/spdlog/spdlog.h"

#define DEBUG spdlog::info

// Main entry point for matching engine. Consider this the "controller"
std::list<Order *> OrderBook::newOrder(Order * order) {
    std::list<Order *> updated_orders;

    DEBUG("Called newOrder");

    if (isOpposingOrderBookBlank(order)) {
        DEBUG("Called isOpposingOrderBookBlank1");

        addOrder(order);
        DEBUG("Called addOrder1");

        adjustBidAskIfOrderIsBetterPrice(order);
        DEBUG("Called adjustBidAskIfOrderIsBetterPrice");

        return updated_orders;
    }

    // If this order has not crossed the spread meaning
    // if it's a buy order it's less than current ask and
    // if a sell order greater than current bid.
    if (!orderCrossedSpread(order)) {
        DEBUG("orderCrossedSpread returned false");

        addOrder(order);
        DEBUG("Called addOrder2");

        return updated_orders;
    }

    // In this implementation, if you are willing to cross the spread, you get the trade.
    // This is changeable to create different trading scenarios.

    // Iteratively attempt to fill the order until we can't.
    // Then insert the rest of the order into the book.
    while (order->unfilled_quantity() > 0) {
        updated_orders.merge(this->fillOrder(order));
        DEBUG("Called fillOrder");

        // If there are no more orders, return.
        if (isOpposingOrderBookBlank(order)) {
            // @TODO should probably wrap bestAsk updates in a setter so we can report changes.
            bestAsk = nullptr;
            return updated_orders;
        }

        // Because we filled some orders, update the best ask if necessary.
        setBidAskToReflectMarket();
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

    orderMap->erase(id);
}

bool OrderBook::orderCrossedSpread(Order* order) {
    if (order->side == BUY) {
        return order->limitPrice >= bestAsk->getPrice();
    } else if (order->side == SELL) {
        return order->limitPrice <= bestBid->getPrice();
    }
}

void OrderBook::adjustBidAskIfOrderIsBetterPrice(Order* order) {
    DEBUG("adjustBidAskIfOrderIsBetterPrice called");

    if (order->side == BUY) {
        // DEBUG("adjustBidAskIfOrderIsBetterPrice BUY");
        // If there are no sell orders & this is a higher bid, move up the bid.
        if (bestBid == nullptr || order->limitPrice > bestBid->getPrice()) {
            // DEBUG("adjustBidAskIfOrderIsBetterPrice comparison for BUY true");
            
            bestBid = buyBook->get(order->limitPrice);
            // DEBUG("adjustBidAskIfOrderIsBetterPrice set bestBid");
        }
    } else if (order->side == SELL) {
        DEBUG("adjustBidAskIfOrderIsBetterPrice SELL");
        // If there are no buy orders & this is a lower ask, lower the ask
        if (bestAsk == nullptr || order->limitPrice < bestAsk->getPrice()) {
            DEBUG("adjustBidAskIfOrderIsBetterPrice comparison for SELL true");

            bestAsk = sellBook->get(order->limitPrice);
            DEBUG("adjustBidAskIfOrderIsBetterPrice set bestAsk");
        }
    }
}

void OrderBook::setBidAskToReflectMarket() {
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

    while (bestBid->getVolume() == 0) {
        PriceLevel * newBestBid = buyBook->get(bestBid->getPrice() - ONE_CENT);
        // sell book get should return null ptr if we retrieve a bad price.
        // or consider adding new prices automagically.
        if (newBestBid != nullptr && newBestBid->getVolume() > 0) {
            bestBid = newBestBid;
        }
    }
}

bool OrderBook::isOpposingOrderBookBlank(Order* order) {
    if (order->side == BUY) {
        return (sellBook == nullptr || sellBook->getVolume() == 0);
    } else if (order->side == SELL) {
        return (buyBook == nullptr || buyBook->getVolume() == 0);
    }
}

void OrderBook::addOrder(Order* order) {
    // DEBUG("Now inside addOrder");

    totalVolume += order->unfilled_quantity();
    // DEBUG("addOrder totalVolume updated");

    Node<Order*> * node;
    if (order->side == BUY) {
        node = buyBook->addOrder(order);
        // DEBUG("buyBook->addOrder(order); called");
    } else if (order->side == SELL) {
        node = sellBook->addOrder(order);
        // DEBUG("sellBook->addOrder(order); called");
    }

    orderMap->emplace(order->id, node);
}

OrderBook::OrderBook() {
    orderMap = new std::unordered_map<SEQUENCE_ID, Node<Order*> * >();
    // @TOOD the book should not care about the min / max prices.
    buyBook = new Book();
    sellBook = new Book();
    bestBid = nullptr;
    bestAsk = nullptr;
}

// Attempts to fill an order using the buy / sell books.
std::list<Order *> OrderBook::fillOrder(Order* order) {
    std::list<Order *> updated_orders;
    int initialQuantity = order->unfilled_quantity();

    if (order->side == BUY) {
        updated_orders = sellBook->fillOrder(order);
    } else if (order->side == SELL) {
        updated_orders = buyBook->fillOrder(order);
    }

    totalVolume -= (initialQuantity - order->unfilled_quantity());
    return updated_orders;
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
