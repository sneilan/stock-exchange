#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../../order_book/order_book.h"
#include "../../order_book/book.h"
#include "../helpers.h"

TEST_CASE("Hello world book test.") {
    Book book;
    book.initPriceDataStructures(0, 10);

    Order * order = createDefaultOrder();
    order->limitPrice = 5;
    book.addOrder(order);

    REQUIRE(book.getVolume() == 100);
}
