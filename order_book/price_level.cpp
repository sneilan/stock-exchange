#include "price_level.h"

// Returns a list of filled orders.
// Quantity of order is modified in place.
// Order is expected to be for opposite side.
std::list<Order *> PriceLevel::fillOrder(Order *order) {
  // Keep popping orders off at price level until we have either filled quantity
  // or run out of orders.

  int unfilled_quantity = order->unfilled_quantity();
  spdlog::debug("PriceLevel::fillOrder::{} - Order id {} unfilled_quantity is "
                "{}. num orders in queue is {}",
                limitPrice, order->id, unfilled_quantity, orders.get_total());

  std::list<Order *> updated_orders;
  while (orders.get_front() != nullptr && unfilled_quantity > 0) {
    Node<Order *> *node = orders.get_front();
    int quantity_available = node->data->unfilled_quantity();
    spdlog::debug("PriceLevel::fillOrder::{} - iterating node {}, quantity {}",
                  limitPrice, node->data->id, quantity_available);

    updated_orders.push_back(node->data);

    if (quantity_available >= unfilled_quantity) {
      spdlog::debug("PriceLevel::fillOrder::{} - Order id {} filling.",
                    limitPrice, order->id);
      node->data->filled_quantity += order->unfilled_quantity();
      totalVolume -= order->unfilled_quantity();
      order->filled_quantity = order->quantity;
      // If we've filled the order, stop.
      spdlog::debug("PriceLevel::fillOrder::{} - Order id {} filled. volume {} "
                    "remains on level. Order has {} quantity remaining.",
                    limitPrice, order->id, totalVolume,
                    order->unfilled_quantity());
      if (node->data->unfilled_quantity() == 0) {
        spdlog::debug("PriceLevel::fillOrder::{} - Order id {} removing node "
                      "{} because order filled.",
                      limitPrice, order->id, node->data->id);
        orders.remove(node);
      }
      break;
    } else if (unfilled_quantity > quantity_available) {
      spdlog::debug("PriceLevel::fillOrder::{} - Order id {} partial filling",
                    limitPrice, order->id);
      order->filled_quantity += quantity_available;
      unfilled_quantity -= quantity_available;
      totalVolume -= quantity_available;
      spdlog::debug("PriceLevel::fillOrder::{} - Order id {} partial filling, "
                    "removing node {}",
                    limitPrice, order->id, node->data->id);
      orders.remove(node);
      spdlog::debug(
          "PriceLevel::fillOrder::{} - Order id {} quantity {} remains",
          limitPrice, order->id, order->unfilled_quantity());
    }
  }

  return updated_orders;
}

void PriceLevel::cancelOrder(Node<Order *> *node) {
  this->totalVolume -= node->data->unfilled_quantity();
  orders.remove(node);
  spdlog::debug("PriceLevel::cancelOrder::{} - Level now has {} orders",
                limitPrice, orders.get_total());
}

Node<Order *> *PriceLevel::addOrder(Order *order) {
  this->totalVolume += order->quantity;
  Node<Order *> *node = orders.push_back(order);
  spdlog::debug("PriceLevel::addOrder::{} - Level now has {} orders",
                limitPrice, orders.get_total());
  return node;
}

int PriceLevel::getVolume() { return totalVolume; }

int PriceLevel::getPrice() { return limitPrice; }

PriceLevel::PriceLevel(PRICE price) {
  limitPrice = price;
  totalVolume = 0;
}
