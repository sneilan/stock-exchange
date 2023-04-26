#ifndef HELPERS_H
#define HELPERS_H

#include "../order_book/order_book.h"

Order * createDefaultOrder() {
    Order * order = new Order();
    order->limitPrice = 5000;
    order->quantity = 100;
    order->side = 'b';
    order->id = 1;
    return order;
}

Order * orderQuantity(int quantity) {
    Order * order = new Order();
    order->limitPrice = 5000;
    order->quantity = quantity;
    order->side = 'b';
    order->id = 1;
    return order;
}

#endif