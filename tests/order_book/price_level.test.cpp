#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../../order_book/order_book.h"
#include "../helpers.h"

TEST_CASE("Price levels - volume") {
    PriceLevel * level = new PriceLevel();
    REQUIRE(level->getVolume() == 0);

    Order * order = createDefaultOrder();
    level->addOrder(order);

    REQUIRE(level->getVolume() == 100);

    Order * order2 = createDefaultOrder();
    level->addOrder(order2);

    REQUIRE(level->getVolume() == 200);

    level->fillOrder(orderQuantity(50));

    REQUIRE(level->getVolume() == 150);

    level->fillOrder(orderQuantity(25));

    REQUIRE(level->getVolume() == 125);

    level->fillOrder(orderQuantity(30));

    REQUIRE(level->getVolume() == 95);

    level->fillOrder(orderQuantity(95));

    REQUIRE(level->getVolume() == 0);
}

TEST_CASE("Price levels - small quantities") {
    PriceLevel * level = new PriceLevel();
    Order * order = createDefaultOrder();
    level->addOrder(order);

    Order * order2 = createDefaultOrder();
    order2->id = 1;
    level->addOrder(order2);

    std::list<Order *> updated_orders = level->fillOrder(orderQuantity(50));

    REQUIRE(updated_orders.size() == 1);
    REQUIRE(order->filled_quantity == 50);
    REQUIRE(order2->filled_quantity == 0);

    std::list<Order *> updated_orders2 = level->fillOrder(orderQuantity(50));
    REQUIRE(updated_orders2.size() == 1);
    REQUIRE(order->filled_quantity == 100);
    REQUIRE(order2->filled_quantity == 0);

    std::list<Order *> updated_orders3 = level->fillOrder(orderQuantity(50));

    REQUIRE(updated_orders3.size() == 1);
    REQUIRE(order->filled_quantity == 100);
    REQUIRE(order2->filled_quantity == 50);
}

TEST_CASE("Price levels - large quantities") {
    PriceLevel * level = new PriceLevel();
    // Create 1000 orders
    for (int i = 0; i < 10; i += 1) {
        level->addOrder(createDefaultOrder());
    }

    // Fill 500 of them.
    std::list<Order * > updated_orders = level->fillOrder(orderQuantity(500));
    REQUIRE(updated_orders.size() == 5);

    REQUIRE(level->getVolume() == 500);
}