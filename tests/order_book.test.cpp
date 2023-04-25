#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../order_book.h"

TEST_CASE("Hello world order book test.") {
    // Hello world order book test. 
    OrderBook* orderBook = new OrderBook();
    Order * order = new Order();
    order->limitPrice = 5000;
    order->quantity = 100;
    order->side = 'b';
    order->id = 0;
    orderBook->newOrder(order);
    orderBook->cancelOrder(order->id);
}
