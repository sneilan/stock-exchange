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

TEST_CASE("Price levels - volume") {
    PriceLevel * level = new PriceLevel();
    REQUIRE(level->getVolume() == 0);

    Order * order = createDefaultOrder();
    level->addOrder(order);

    REQUIRE(level->getVolume() == 100);

    Order * order2 = createDefaultOrder();
    level->addOrder(order2);

    REQUIRE(level->getVolume() == 200);

    level->fillQuantity(50);

    REQUIRE(level->getVolume() == 150);

    level->fillQuantity(25);

    REQUIRE(level->getVolume() == 125);

    level->fillQuantity(30);

    REQUIRE(level->getVolume() == 95);

    level->fillQuantity(95);

    REQUIRE(level->getVolume() == 0);
}

TEST_CASE("Price levels - small quantities") {
    PriceLevel * level = new PriceLevel();
    Order * order = createDefaultOrder();
    level->addOrder(order);

    Order * order2 = createDefaultOrder();
    order2->id = 1;
    level->addOrder(order2);

    std::list<Order *> updated_orders = level->fillQuantity(50);

    REQUIRE(updated_orders.size() == 1);
    REQUIRE(order->filled_quantity == 50);
    REQUIRE(order2->filled_quantity == 0);

    std::list<Order *> updated_orders2 = level->fillQuantity(50);
    REQUIRE(updated_orders2.size() == 1);
    REQUIRE(order->filled_quantity == 100);
    REQUIRE(order2->filled_quantity == 0);

    std::list<Order *> updated_orders3 = level->fillQuantity(50);

    REQUIRE(updated_orders3.size() == 1);
    REQUIRE(order->filled_quantity == 100);
    REQUIRE(order2->filled_quantity == 50);
}

TEST_CASE("Price levels - large quantities") {
    PriceLevel * level = new PriceLevel();
    for (int i = 0; i < 10; i += 1) {
        level->addOrder(createDefaultOrder());
    }

    level->fillQuantity(500);

    REQUIRE(level->getVolume() == 500);
}