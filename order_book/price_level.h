#ifndef price_level_h
#define price_level_h
#include "../eventstore.h"
#include "../util/linked_list.h"
#include "../util/types.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <stdexcept>
#include <unordered_map>

/* Keeps list of orders for each price */
class PriceLevel {
public:
  PriceLevel(PRICE price);
  Node<Order *> *addOrder(Order *order);
  std::list<Order *> fillOrder(Order *order);
  void cancelOrder(Node<Order *> *node);
  int getVolume();
  int getPrice();

private:
  // Prices are stored in pennies. $4.56 = 456.
  PRICE limitPrice;
  long totalVolume;
  DoublyLinkedList<Order *> orders;
};

#endif