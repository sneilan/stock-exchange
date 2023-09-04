#include "../order_book/order_book.h"

#include "helpers.h"

#define DEFAULT_PRICE 5000
#define DEFAULT_QUANTITY 100
#define DEFAULT_SIDE 'b'

// Create a logical sequence clock for testing purposes
int sequence_id = 1;

Order *createDefaultOrder() {
  Order *order = new Order();
  order->limitPrice = DEFAULT_PRICE;
  order->quantity = DEFAULT_QUANTITY;
  order->side = DEFAULT_SIDE;
  order->id = sequence_id;
  sequence_id++;
  return order;
}

Order *orderQuantity(int quantity) {
  Order *order = new Order();
  order->limitPrice = DEFAULT_PRICE;
  order->quantity = quantity;
  order->side = DEFAULT_SIDE;
  order->id = sequence_id;
  sequence_id++;
  return order;
}

Order *customOrder(int price, int quantity, char side) {
  Order *order = new Order();
  order->limitPrice = price;
  order->quantity = quantity;
  order->side = side;
  order->id = sequence_id;
  sequence_id++;
  return order;
}
