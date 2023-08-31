#ifndef HELPERS_H
#define HELPERS_H

#include "../order_book/order_book.h"

Order * createDefaultOrder();
Order * customOrder(int price, int quantity, char side);

Order * orderQuantity(int quantity);

#endif
