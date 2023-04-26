#include "price_level.h"

// Returns a list of filled orders.
// Quantity of order is modified in place.
std::list<Order *> PriceLevel::fillOrder(Order* order) {
    // Keep popping orders off at price level until we have either filled quantity
    // or run out of orders.

    int requested_quantity = order->unfilled_quantity();

    std::list<Order *> updated_orders;
    while (orders.get_front() != nullptr && requested_quantity > 0) {
        Node<Order *> * node = orders.get_front();
        int quantity_available = node->data->unfilled_quantity();

        updated_orders.push_back(node->data);

        if (quantity_available > requested_quantity) {
            node->data->filled_quantity += requested_quantity;
            totalVolume -= requested_quantity;
            requested_quantity = 0;
            // If we've filled the order, stop.
            break;
        } else if (requested_quantity >= quantity_available) {
            node->data->filled_quantity += quantity_available;
            requested_quantity -= quantity_available;
            totalVolume -= quantity_available;
            orders.remove(node);
        }
    }

    order->filled_quantity += (order->unfilled_quantity() - requested_quantity);

    return updated_orders;
}

void PriceLevel::cancelOrder(Node<Order*> * node) {
    totalVolume -= node->data->unfilled_quantity();
    orders.remove(node);
}

Node<Order *> * PriceLevel::addOrder(Order* order) {
    totalVolume += order->quantity;
    return orders.push_back(order);
}

int PriceLevel::getVolume() {
    return totalVolume;
}

int PriceLevel::getPrice() {
    return limitPrice;
}
