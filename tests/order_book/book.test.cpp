#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../../order_book/book.h"
#include "../../order_book/order_book.h"
#include "../helpers.h"

TEST_CASE("Book test - Add & cancel order") {
  Book book;
  book.initPriceDataStructures(0, 10);

  Order *order = createDefaultOrder();
  order->limitPrice = 5;
  Node<Order *> *node = book.addOrder(order);

  REQUIRE(book.getVolume() == 100);

  book.cancelOrder(node);

  REQUIRE(book.getVolume() == 0);
}

TEST_CASE("Book test - fill order") {
  Book book;
  book.initPriceDataStructures(0, 10);

  // Create an order on this side.
  Order *order = createDefaultOrder();
  order->limitPrice = 5000;
  Node<Order *> *node = book.addOrder(order);
  REQUIRE(node->data->filled_quantity == 0);
  REQUIRE(book.get(order->limitPrice)->getVolume() == 100);

  // Create a new order and attempt to fill it.
  Order *oppositeOrder = orderQuantity(50);
  REQUIRE(order->limitPrice == oppositeOrder->limitPrice);
  REQUIRE(book.get(oppositeOrder->limitPrice)->getVolume() == 100);
  PriceLevel *level = book.get(oppositeOrder->limitPrice);

  std::list<Order *> updated_orders = book.fillOrder(oppositeOrder, level);

  REQUIRE(book.getVolume() == 50);

  REQUIRE(book.fillOrder(orderQuantity(5), level).size() == 1);
}
