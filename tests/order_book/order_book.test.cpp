#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../../order_book/order_book.h"
#include "../helpers.h"

TEST_CASE("Hello world order book test.") {
    // Hello world order book test.
    /*
    OrderBook* orderBook = new OrderBook();
    Order * order = createDefaultOrder();

    orderBook->newOrder(order);
    REQUIRE(orderBook->getVolume() == 100);

    orderBook->cancelOrder(order->id);
    REQUIRE(orderBook->getVolume() == 0);
    */
}

TEST_CASE("Adjusting best bid and ask.") {
    // @TODO
}
