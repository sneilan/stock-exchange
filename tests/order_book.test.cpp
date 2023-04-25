#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../order_book.h"
#include "helpers.h"

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

TEST_CASE("Price level fills unit test") {
    PriceLevel * level = new PriceLevel();
    Order * order = createDefaultOrder();
    level->addOrder(order);

    Order * order2 = createDefaultOrder();
    order2->id = 1;
    level->addOrder(order2);

    REQUIRE(level->getVolume() == 200);

    std::list<Order *> updated_orders = level->fillQuantity(50);
    REQUIRE(level->getVolume() == 150);

}
