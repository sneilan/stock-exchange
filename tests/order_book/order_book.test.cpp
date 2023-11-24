#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../../src/order_book/order_book.h"
#include "../helpers.h"

#include <spdlog/spdlog.h>

#define DEBUG spdlog::info

TEST_CASE("order_book - add order") {
  // Hello world order book test.
  OrderBook *orderBook = new OrderBook(nullptr);

  Order *order = createDefaultOrder();
  orderBook->newOrder(order);

  REQUIRE(orderBook->getVolume() == 100);
}

TEST_CASE("order_book - cancel order") {
  OrderBook *orderBook = new OrderBook(nullptr);
  Order *order = createDefaultOrder();

  orderBook->newOrder(order);

  REQUIRE(orderBook->getVolume() == 100);

  orderBook->cancelOrder(order->id);

  REQUIRE(orderBook->getVolume() == 0);
}

TEST_CASE("order_book - new orders that do not fill should set bid/ask price") {
  OrderBook *orderBook = new OrderBook(nullptr);

  // Making a buy order w/ no sell order should immediately set bid.
  Order *buyOrder = createDefaultOrder();
  orderBook->newOrder(buyOrder);

  REQUIRE(orderBook->getBid()->getPrice() == buyOrder->limitPrice);

  // A sell order that does not cross the spread should now set the ask price.
  Order *sellOrder = createDefaultOrder();
  sellOrder->side = SELL;
  sellOrder->limitPrice = buyOrder->limitPrice + 100;
  orderBook->newOrder(sellOrder);

  REQUIRE(orderBook->getAsk()->getPrice() == sellOrder->limitPrice);
}

TEST_CASE("order_book - match order") {
  OrderBook *orderBook = new OrderBook(nullptr);

  Order *buyOrder = createDefaultOrder();
  orderBook->newOrder(buyOrder);
  REQUIRE(orderBook->getBid()->getPrice() == buyOrder->limitPrice);

  Order *sellOrder = createDefaultOrder();
  sellOrder->side = SELL;
  orderBook->newOrder(sellOrder);

  REQUIRE(orderBook->getVolume() == 0);
}

TEST_CASE("order_book - buy orders with higher prices should move bid up") {
  OrderBook *orderBook = new OrderBook(nullptr);

  Order *buyOrder = createDefaultOrder();
  orderBook->newOrder(buyOrder);
  REQUIRE(orderBook->getBid()->getPrice() == buyOrder->limitPrice);

  Order *buyOrderHigher = createDefaultOrder();
  buyOrderHigher->limitPrice = buyOrder->limitPrice + 100;
  orderBook->newOrder(buyOrderHigher);
  REQUIRE(orderBook->getBid()->getPrice() == buyOrderHigher->limitPrice);
}

TEST_CASE("order_book - sell orders with lower prices should move ask lower") {
  OrderBook *orderBook = new OrderBook(nullptr);

  Order *sellOrder = createDefaultOrder();
  sellOrder->side = SELL;
  orderBook->newOrder(sellOrder);
  REQUIRE(orderBook->getAsk()->getPrice() == sellOrder->limitPrice);

  Order *sellOrderLower = createDefaultOrder();
  sellOrderLower->side = SELL;
  sellOrderLower->limitPrice = sellOrder->limitPrice - 100;
  orderBook->newOrder(sellOrderLower);
  REQUIRE(orderBook->getAsk()->getPrice() == sellOrderLower->limitPrice);
}

TEST_CASE("order_book - testing order fills after order book populated") {
  OrderBook *orderBook = new OrderBook(nullptr);

  // initial buy order
  Order *order1 = customOrder(100, 100, 'b');
  orderBook->newOrder(order1);

  REQUIRE(orderBook->getVolume() == 100);

  // sell order that does not cross spread.
  Order *order2 = customOrder(110, 100, 's');
  orderBook->newOrder(order2);

  REQUIRE(orderBook->getVolume() == 200);

  // buy order that also does not cross spread.
  Order *order3 = customOrder(105, 100, 'b');
  orderBook->newOrder(order3);

  REQUIRE(orderBook->getVolume() == 300);

  // sell order that should match only with buy order for 105 and not 100.
  Order *order4 = customOrder(103, 100, 's');
  orderBook->newOrder(order4);

  REQUIRE(orderBook->getVolume() == 200);
}

TEST_CASE("order_book - testing fillOrder when we attempt to sell more than "
          "what is offered.") {
  OrderBook *orderBook = new OrderBook(nullptr);

  // initial buy order.
  Order *order1 = customOrder(336, 180, 'b');
  orderBook->newOrder(order1);

  REQUIRE(orderBook->getVolume() == 180);

  // sell order that does not cross spread.
  Order *order2 = customOrder(698, 170, 's');
  orderBook->newOrder(order2);

  REQUIRE(orderBook->getVolume() == 170 + 180);

  // buy order that does not adjust best bid and does not cross spread.
  Order *order3 = customOrder(126, 130, 'b');
  orderBook->newOrder(order3);

  REQUIRE(orderBook->getVolume() == 170 + 180 + 130);

  // Sell order that crosses spread and matches with buy order for 336.
  // However the order has one more unit than we have available.
  // exchange should know to give up.
  Order *order4 = customOrder(180, 181, 's');
  REQUIRE(order4->unfilled_quantity() == 181);
  orderBook->newOrder(order4);
  REQUIRE(order4->unfilled_quantity() == 0);

  REQUIRE(orderBook->getBid()->getPrice() == 126);
}
