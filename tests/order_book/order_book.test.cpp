#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../../order_book/order_book.h"
#include "../helpers.h"

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

TEST_CASE("order_book - fill order") {
    OrderBook* orderBook = new OrderBook();
    Order * buyOrder = createDefaultOrder();

    orderBook->newOrder(buyOrder);

    Order * sellOrder = createDefaultOrder();
    sellOrder->side = SELL;
}