#ifndef HELPERS_H
#define HELPERS_H

#include "../order_book.h"

Order * createDefaultOrder() {
    Order * order = new Order();
    order->limitPrice = 5000;
    order->quantity = 100;
    order->side = 'b';
    order->id = 1;
    return order;
}

#endif