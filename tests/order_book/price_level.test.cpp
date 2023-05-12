#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../../order_book/order_book.h"
#include "../helpers.h"

TEST_CASE("Price levels - filling full orders") {
    Order * order = createDefaultOrder();
    PriceLevel * level = new PriceLevel(order->limitPrice);

    REQUIRE(level->getVolume() == 0);

    level->addOrder(order);

    REQUIRE(level->getVolume() == 100);

    Order * order2 = createDefaultOrder();
    level->addOrder(order2);

    REQUIRE(level->getVolume() == 200);

    level->fillOrder(orderQuantity(200));

    REQUIRE(level->getVolume() == 0);
}

TEST_CASE("Price levels - filling partial orders") {
    Order * order = createDefaultOrder();
    PriceLevel * level = new PriceLevel(order->limitPrice);

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
