#include "order_book.h"

OrderBook::OrderBook() {
    orderMap = new std::unordered_map<SEQUENCE_ID, Node<Order*> * >();
    buyBook = new Book();
    sellBook = new Book();
    bestBid = nullptr;
    bestAsk = nullptr;
}

std::list<Order *> OrderBook::newOrder(Order * order) {
    totalVolume += order->unfilled_quantity();
    std::list<Order *> updated_orders;

    // When a new buy order comes in that becomes a bid.
    if (order->side == BUY) {
        // If there is an asking price & there are sellers in the market.
        if (sellBook != nullptr) {
            if (sellBook->getVolume() > 0) {
                // Attempt to fill the order.
                // @TODO report these filled asks to the users.
                // @TODO should I be passing the whole order to fillQuantity so it can update the order?
                // Operating on the ask directly causes an issue with updating open interest across the stack.
                // before filling an order, is this order at the limit price or better? if not, 
                // simply put into buy book & move on.
                std::list<Order *> filled_asks = bestAsk->fillOrder(order);
                // @TODO update Book / OrderBook volume.
                // If there is still unfilled quantity, move the best ask around & try again.
                if (order->unfilled_quantity() > 0) {
                    if (sellBook->getVolume() > 0) {
                        // Will there be sells below the bid price?
                        // No because if you are selling below the bid price your order will be immediately filled anyway.
                        PriceLevel * newBestAsk = sellBook->get(bestAsk->getPrice() + ONE_CENT);
                        // sell book get should return null ptr if we retrieve a bad price.
                        // or consider adding new prices automagically.
                        if (newBestAsk != nullptr && newBestAsk->getVolume() > 0) {
                            bestAsk = newBestAsk;
                            // now that we have a new best ask, fill the order
                        }
                    } else {

                    }
                }
            }
        }

        if (bestBid == nullptr) {
            bestBid = buyBook->get(order->limitPrice);
            auto *ptr = buyBook->insert(order);
            orderMap->insert(std::make_pair(order->id, ptr));
            // @TODO do I also return the users order itself?
            return updated_orders;
        }

        // First try to fill the order at the best available offer.

        // If there are any shares remaining, try to fill them at the next available price if volume > 0

        // If we run out of bids or whatever at this level, move best bid around.
        // if best bid becomes null b/c no orders then it's just whatever it is.

        // if buying, try to fill in orders
        // move the bids / asks around.
        // If this buy price is lo
        //if (order->limitPrice < bestBid->getPrice()) {
          //  bestBid = buyBook->get(order->limitPrice);
        //}
    } else {
        auto *ptr = sellBook->insert(order);
        orderMap->insert(std::make_pair(order->id, ptr));

        // if (order->limitPrice > bestOffer->getPrice()) {
           // bestOffer = sellBook->get(order->limitPrice);
        // }
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

int OrderBook::getVolume() {
    return totalVolume;
}

PriceLevel* OrderBook::getBid() {
    return bestBid;
}

PriceLevel* OrderBook::getAsk() {
    return bestAsk;
}