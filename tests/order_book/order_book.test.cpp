#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../../order_book/order_book.h"
#include "../helpers.h"

#include "../util/spdlog/spdlog.h"

#define DEBUG spdlog::info

TEST_CASE("order_book - add order") {
    // Hello world order book test.
    OrderBook* orderBook = new OrderBook();

    Order * order = createDefaultOrder();
    orderBook->newOrder(order);

    REQUIRE(orderBook->getVolume() == 100);
}

TEST_CASE("order_book - cancel order") {
    OrderBook* orderBook = new OrderBook();
    Order * order = createDefaultOrder();

    orderBook->newOrder(order);
    orderBook->cancelOrder(order->id);
}

TEST_CASE("order_book - new orders that do not fill should set bid/ask price") {
    OrderBook* orderBook = new OrderBook();

    // Making a buy order w/ no sell order should immediately set bid.
    Order * buyOrder = createDefaultOrder();
    orderBook->newOrder(buyOrder);

    REQUIRE(orderBook->getBid()->getPrice() == buyOrder->limitPrice);

    // A sell order that does not cross the spread should now set the ask price.
    Order * sellOrder = createDefaultOrder();
    sellOrder->side = SELL;
    sellOrder->limitPrice = buyOrder->limitPrice + 100;
    orderBook->newOrder(sellOrder);

    REQUIRE(orderBook->getAsk()->getPrice() == sellOrder->limitPrice);
}

/*
TEST_CASE("order_book - match order") {
    OrderBook* orderBook = new OrderBook();

    Order * buyOrder = createDefaultOrder();
    orderBook->newOrder(buyOrder);
    REQUIRE(orderBook->getBid()->getPrice() == buyOrder->limitPrice);

    Order * sellOrder = createDefaultOrder();
    sellOrder->side = SELL;
    orderBook->newOrder(sellOrder);

    REQUIRE(orderBook->getVolume() == 0);
}
*/
