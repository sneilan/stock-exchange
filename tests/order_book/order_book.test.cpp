#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../../order_book/order_book.h"
#include "../helpers.h"

#include <spdlog/spdlog.h>

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

    REQUIRE(orderBook->getVolume() == 100);

    orderBook->cancelOrder(order->id);

    REQUIRE(orderBook->getVolume() == 0);
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

TEST_CASE("order_book - buy orders with higher prices should move bid up") {
    OrderBook* orderBook = new OrderBook();

    Order * buyOrder = createDefaultOrder();
    orderBook->newOrder(buyOrder);
    REQUIRE(orderBook->getBid()->getPrice() == buyOrder->limitPrice);

    Order * buyOrderHigher = createDefaultOrder();
    buyOrderHigher->limitPrice = buyOrder->limitPrice + 100;
    orderBook->newOrder(buyOrderHigher);
    REQUIRE(orderBook->getBid()->getPrice() == buyOrderHigher->limitPrice);
}

TEST_CASE("order_book - sell orders with lower prices should move ask lower") {
    OrderBook* orderBook = new OrderBook();

    Order * sellOrder = createDefaultOrder();
    sellOrder->side = SELL;
    orderBook->newOrder(sellOrder);
    REQUIRE(orderBook->getAsk()->getPrice() == sellOrder->limitPrice);

    Order * sellOrderLower = createDefaultOrder();
    sellOrderLower->side = SELL;
    sellOrderLower->limitPrice = sellOrder->limitPrice - 100;
    orderBook->newOrder(sellOrderLower);
    REQUIRE(orderBook->getAsk()->getPrice() == sellOrderLower->limitPrice);
}

TEST_CASE("order_book - odd lots") {
    OrderBook* orderBook = new OrderBook();

    // Order * order1 = customOrder(100, 100, 'b');
    // orderBook->newOrder(order1);
    // Order * order2 = customOrder(300, 100, 's');
    // orderBook->newOrder(order2);
    // Order * order3 = customOrder(200, 100, 'b');
    // orderBook->newOrder(order3);
    // Order * order4 = customOrder(150, 100, 's');
    // orderBook->newOrder(order4);


    Order * order1 = customOrder(100, 100, 'b');
    orderBook->newOrder(order1);
    Order * order2 = customOrder(110, 100, 's');
    orderBook->newOrder(order2);
    Order * order3 = customOrder(105, 100, 'b');
    orderBook->newOrder(order3);
    Order * order4 = customOrder(103, 100, 's');
    orderBook->newOrder(order4);

}
