#include "../order_book/order_book.h"

#include "helpers.h"

#define DEFAULT_PRICE 5000
#define DEFAULT_QUANTITY 100
#define DEFAULT_SIDE 'b'

Order * createDefaultOrder() {
    Order * order = new Order();
    order->limitPrice = DEFAULT_PRICE;
    order->quantity = DEFAULT_QUANTITY;
    order->side = DEFAULT_SIDE;
    order->id = 1;
    return order;
}

Order * orderQuantity(int quantity) {
    Order * order = new Order();
    order->limitPrice = DEFAULT_PRICE;
    order->quantity = quantity;
    order->side = DEFAULT_SIDE;
    order->id = 1;
    return order;
}
