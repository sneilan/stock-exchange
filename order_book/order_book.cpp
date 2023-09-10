#include "order_book.h"
#include <spdlog/spdlog.h>
#include <sstream>

// Main entry point for matching engine. Consider this the "controller"
std::list<Order *> OrderBook::newOrder(Order *order) {
  std::list<Order *> updated_orders;

  SPDLOG_INFO("Called newOrder on Order {} side {} price {} quantity {}",
               order->id, order->side, order->limitPrice,
               order->unfilled_quantity());

  if (isOpposingOrderBookBlank(order)) {
    SPDLOG_DEBUG("isOpposingOrderBookBlank returned true");

    addOrder(order);
    SPDLOG_DEBUG("addOrder called");

    adjustBidAskIfOrderIsBetterPrice(order);
    SPDLOG_DEBUG("adjustBidAskIfOrderIsBetterPrice called");

    SPDLOG_DEBUG("opposingOrderBook volume is {}", opposingOrderVolume(order));
    SPDLOG_DEBUG("orderBook volume is {}", bookOrderVolume(order));

    return updated_orders;
  }

  // If this order has not crossed the spread meaning
  // if it's a buy order it's less than current ask and
  // if a sell order greater than current bid.
  // It's like saying a user does not want to buy at a price
  // anyone wants to sell at and does not want to sell at a price
  // anyone wants to buy at. For example, say the bid (highest price
  // buyer will pay) is $5.00 but I'm only willing to sell for $6.00.
  // This order would not cross the spread because the buyer isn't willing to
  // pay $6.00. If I changed my order to $4.50, this would cross the spread
  // because the buyer is willing the pay $5.00 and I'm willing to sell for
  // $4.50 so that's a match.
  if (!orderCrossedSpread(order)) {
    SPDLOG_DEBUG("orderCrossedSpread returned false");

    addOrder(order);

    adjustBidAskIfOrderIsBetterPrice(order);

    SPDLOG_DEBUG("opposingOrderBook volume is {}", opposingOrderVolume(order));
    SPDLOG_DEBUG("orderBook volume is {}", bookOrderVolume(order));

    return updated_orders;
  }

  // In this implementation, if you are willing to cross the spread, you get the
  // trade. This is changeable to create different trading scenarios.

  // Iteratively attempt to fill the order until we can't.
  // Then insert the rest of the order into the book.
  SPDLOG_DEBUG("unfilledQuantity on order is {}", order->unfilled_quantity());
  SPDLOG_DEBUG("opposingOrderBook volume is {}", opposingOrderVolume(order));
  SPDLOG_DEBUG("orderBook volume is {}", bookOrderVolume(order));
  while (order->unfilled_quantity() > 0) {
    updated_orders.merge(this->fillOrder(order));
    SPDLOG_DEBUG("Finished fillOrder. Order id {} has unfillled quantity {}",
                  order->id, order->unfilled_quantity());

    SPDLOG_DEBUG("opposingOrderBook volume is {}", opposingOrderVolume(order));
    SPDLOG_DEBUG("orderBook volume is {}", bookOrderVolume(order));

    // Because we filled some orders, update the best ask if necessary.
    setBidAskToReflectMarket();
    SPDLOG_DEBUG("Called set bid ask");

    // If there are no more orders, break.
    if (isOpposingOrderBookBlank(order)) {
      SPDLOG_DEBUG("Opposing order book blank");
      break;
    }
    printBestBidAsk("fillOrder requested info. ");

    // If user is looking to buy but no-one is willing to sell as low as they
    // want to buy then break because even though we have opposing orders,
    // nothing will get filled. People try to sell for as high as possible but
    // buy for as low as possible. So we want to see if the ask is less than
    // what user is attempting to buy for.
    if (order->side == BUY && bestAsk->getPrice() < order->limitPrice) {
      break;
    }

    // and vice versa.
    if (order->side == SELL && bestBid->getPrice() > order->limitPrice) {
      break;
    }
  }

  return updated_orders;
}

void OrderBook::cancelOrder(SEQUENCE_ID id) {
  Node<Order *> *node = orderMap->at(id);
  totalVolume -= node->data->unfilled_quantity();

  Order *order = node->data;

  if (order->side == BUY) {
    buyBook->cancelOrder(node);
  } else if (order->side == SELL) {
    sellBook->cancelOrder(node);
  }

  orderMap->erase(id);
}

bool OrderBook::orderCrossedSpread(Order *order) {
  if (order->side == BUY) {
    return order->limitPrice >= bestAsk->getPrice();
  }

  // Order is sell side.
  return order->limitPrice <= bestBid->getPrice();
}

void OrderBook::adjustBidAskIfOrderIsBetterPrice(Order *order) {
  SPDLOG_DEBUG("Order id {}", order->id);

  if (order->side == BUY) {
    // If there are no sell orders & this is a higher bid, move up the bid.
    if (bestBid == nullptr || order->limitPrice > bestBid->getPrice()) {

      bestBid = buyBook->get(order->limitPrice);
      SPDLOG_DEBUG("bid is {}/{}",
                    bestBid->getPrice(), bestBid->getVolume());
    }
  } else if (order->side == SELL) {
    // If there are no buy orders & this is a lower ask, lower the ask
    if (bestAsk == nullptr || order->limitPrice < bestAsk->getPrice()) {

      bestAsk = sellBook->get(order->limitPrice);
      SPDLOG_DEBUG("ask is {}/{}",
                    bestAsk->getPrice(), bestAsk->getVolume());
    }
  }
}

void OrderBook::printBestBidAsk(const char *prefix) {
  std::stringstream ss;

  ss << prefix << " ";

  ss << "bid is ";
  if (bestBid != nullptr) {
    ss << bestBid->getPrice() << "/" << bestBid->getVolume() << ". ask is ";
  } else {
    ss << "null/null.";
  }

  ss << "ask is ";
  if (bestAsk != nullptr) {
    ss << bestAsk->getPrice() << "/" << bestAsk->getVolume();
  } else {
    ss << "null/null";
  }

  SPDLOG_DEBUG(ss.str());
}

void OrderBook::setBidAskToReflectMarket() {
  printBestBidAsk("setBidAskToReflectMarket");

  // If we ran out of orders at this price level,
  // Find the next best selling price & make that the ask.
  while (bestAsk != nullptr && bestAsk->getVolume() == 0) {
    int nextPrice = bestAsk->getPrice() + ONE_CENT;

    if (nextPrice > ONE_HUNDRED_DOLLARS) {
      bestAsk = nullptr;
      SPDLOG_DEBUG("ask is null/null");
      return;
    }

    bestAsk = sellBook->get(nextPrice);

    // sell book get should return null ptr if we retrieve a bad price.
    // or consider adding new prices automagically.
    if (bestAsk != nullptr && bestAsk->getVolume() > 0) {
      SPDLOG_DEBUG("ask is {}/{}",
                    bestAsk->getPrice(), bestAsk->getVolume());
      return;
    }
  }

  while (bestBid != nullptr && bestBid->getVolume() == 0) {
    int nextPrice = bestBid->getPrice() - ONE_CENT;
    if (nextPrice < ONE_DOLLAR) {
      SPDLOG_DEBUG("bid is null/null");
      bestBid = nullptr;
      return;
    }

    bestBid = buyBook->get(nextPrice);
    // sell book get should return null ptr if we retrieve a bad price.
    // or consider adding new prices automagically.
    if (bestBid != nullptr && bestBid->getVolume() > 0) {
      SPDLOG_DEBUG("bid is {}/{}",
                    bestBid->getPrice(), bestBid->getVolume());
      return;
    }
  }
}

bool OrderBook::isOpposingOrderBookBlank(Order *order) {
  if (order->side == BUY) {
    return (sellBook == nullptr || sellBook->getVolume() == 0);
  }

  // order is sell side.
  return (buyBook == nullptr || buyBook->getVolume() == 0);
}

int OrderBook::bookOrderVolume(Order *order) {
  if (order->side == BUY) {
    if (buyBook == nullptr) {
      return 0;
    }
    return buyBook->getVolume();
  }

  // order is sell side.
  if (sellBook == nullptr) {
    return 0;
  }
  return sellBook->getVolume();
}

int OrderBook::opposingOrderVolume(Order *order) {
  if (order->side == BUY) {
    if (sellBook == nullptr) {
      return 0;
    }
    return sellBook->getVolume();
  }

  // order is sell side.
  if (buyBook == nullptr) {
    return 0;
  }
  return buyBook->getVolume();
}

void OrderBook::addOrder(Order *order) {
  totalVolume += order->unfilled_quantity();
  SPDLOG_DEBUG("totalVolume is now {}", totalVolume);

  Node<Order *> *node;
  if (order->side == BUY) {
    node = buyBook->addOrder(order);
  } else if (order->side == SELL) {
    node = sellBook->addOrder(order);
  }

  orderMap->emplace(order->id, node);
}

OrderBook::OrderBook() {
  orderMap = new std::unordered_map<SEQUENCE_ID, Node<Order *> *>();
  // @TOOD the book should not care about the min / max prices.
  buyBook = new Book();
  sellBook = new Book();
  bestBid = nullptr;
  bestAsk = nullptr;
}

// Attempts to fill an order using the buy / sell books.
std::list<Order *> OrderBook::fillOrder(Order *order) {
  std::list<Order *> updated_orders;
  int initialQuantity = order->unfilled_quantity();

  if (order->side == BUY) {
    if (bestAsk == nullptr) {
      SPDLOG_DEBUG("bestAsk is null. returning");
      return updated_orders;
    }
    updated_orders = sellBook->fillOrder(order, bestAsk);
  } else if (order->side == SELL) {
    if (bestBid == nullptr) {
      SPDLOG_DEBUG("bestBid is null. returning");
      return updated_orders;
    }
    updated_orders = buyBook->fillOrder(order, bestBid);
  }

  totalVolume -= (initialQuantity - order->unfilled_quantity());
  return updated_orders;
}

int OrderBook::getVolume() { return totalVolume; }

PriceLevel *OrderBook::getBid() { return bestBid; }

PriceLevel *OrderBook::getAsk() { return bestAsk; }
